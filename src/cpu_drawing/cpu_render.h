#include <thread>
#include <vector>
#include <algorithm>
#include <glsl_generated/generated_big_math.h>
#include <glsl_generated/generated_math_mapper.h>
#include <array>
#include <interactions/interactions.h>
#include <graphics/ui.h>
#include <types/types.h>
#include <types/type_mapper.h>
#include <glm/common.hpp>

thread_local std::vector<big_vec2> reg_eval_stack(512);
thread_local size_t reg_sp;
thread_local big_vec2 reg_operand_a;
thread_local big_vec2 reg_operand_b;
thread_local big_vec2 reg_math_res;

void evaluate_ast_stack(
    const std::vector<TokenOperator>& tokens,
    const big_vec2& z_in,
    const CPU_Interpreter& interpreter,
    big_vec2& out_result
) {
    reg_sp = 0;

    for (const TokenOperator& token : tokens) {

        const Operator curr_op = token.op;
        if (curr_op == Operator::VARIABLEZ) {
            reg_eval_stack[reg_sp] = z_in;
            reg_sp++;
        }
        else if (curr_op == Operator::CONSTANT) {
            const glm::vec2 original_vec = token.value;
            reg_eval_stack[reg_sp] = big_vec2(original_vec.x, original_vec.y);
            reg_sp++;
        }
        else if (token.arity == Arity::UNARY) {
            reg_sp--;
            reg_operand_a = reg_eval_stack[reg_sp];

            reg_math_res = interpreter.unary_ops.at(token.op)(reg_operand_a);

            reg_eval_stack[reg_sp] = reg_math_res;
            reg_sp++;
        }
        else if (token.arity == Arity::BINARY) {
            reg_sp--;
            reg_operand_b = reg_eval_stack[reg_sp];

            reg_sp--;
            reg_operand_a = reg_eval_stack[reg_sp];

            reg_math_res = interpreter.binary_ops.at(token.op)(reg_operand_a, reg_operand_b);

            reg_eval_stack[reg_sp] = reg_math_res;
            reg_sp++;
        }
    }
    reg_sp--;
    out_result = reg_eval_stack[reg_sp];
}

inline const big_float HALF = big_float("0.5");

void convert_coordinates(big_vec2& z, const ViewState* view_state) {
    big_float w(view_state->hp_width);
    big_float h(view_state->hp_height);
    
    z = view_state->hp_range * (z - HALF * big_vec2(w, h)) / h;
    z = z + big_vec2(view_state->hp_shift.x, view_state->hp_shift.y);
}

std::array<uint8_t, 3> domain_color(const big_vec2& z) {
    big_float angle_bf = boost::multiprecision::atan2(z.y, z.x);
    big_float mag_bf = length(z);

    double angle = static_cast<double>(angle_bf);
    double mag = static_cast<double>(mag_bf);

    double hue = angle / (2.0 * 3.14159265358979323846);
    if (hue < 0.0) hue += 1.0;

    double light = (2.0 / 3.14159265358979323846) * std::atan(mag);
    double sat = 1.0;

    auto glsl_mod = [](double x, double y) { return x - y * std::floor(x / y); };

    double r_t = std::clamp(std::abs(glsl_mod(hue * 6.0 + 0.0, 6.0) - 3.0) - 1.0, 0.0, 1.0);
    double g_t = std::clamp(std::abs(glsl_mod(hue * 6.0 + 4.0, 6.0) - 3.0) - 1.0, 0.0, 1.0);
    double b_t = std::clamp(std::abs(glsl_mod(hue * 6.0 + 2.0, 6.0) - 3.0) - 1.0, 0.0, 1.0);

    double chroma = 1.0 - std::abs(2.0 * light - 1.0);

    double r = light + sat * (r_t - 0.5) * chroma;
    double g = light + sat * (g_t - 0.5) * chroma;
    double b = light + sat * (b_t - 0.5) * chroma;

    return std::array<uint8_t, 3>{static_cast<uint8_t>(r * 255.0), static_cast<uint8_t>(g * 255.0), static_cast<uint8_t>(b * 255.0)};
}

void render_band(int start_y, int end_y, int width, int height, unsigned char* pixel_buffer, const CPU_Interpreter& interpreter, const std::vector<TokenOperator>& stack, const ViewState* view_state, std::atomic<int>& rows_completed) {
    for (int y = start_y; y < end_y; ++y) {
        for (int x = 0; x < width; ++x) {
            
            big_vec2 z(big_float(x), big_float(height - y));
            
            convert_coordinates(z, view_state);
            
            big_vec2 result;
            evaluate_ast_stack(stack, z, interpreter, result);

            std::array<uint8_t, 3> color_val = domain_color(result);

            int index = (y * width + x) * 4;
            pixel_buffer[index + 0] = color_val[0];
            pixel_buffer[index + 1] = color_val[1];
            pixel_buffer[index + 2] = color_val[2];
            pixel_buffer[index + 3] = 255;
        }
        rows_completed++;
    }
}

void dispatch_render(int width, int height, unsigned char* pixel_buffer, CPU_Interpreter& interpreter, int user_thread_limit, std::vector<TokenOperator> stack, const ViewState* view_state, std::atomic<bool>& is_rendering, std::atomic<int>& rows_completed) {
    if (is_rendering) return;

    is_rendering = true;
    rows_completed = 0;

    std::thread([=, &interpreter, &is_rendering, &rows_completed]() {
        int hardware_threads = std::thread::hardware_concurrency();
        if (hardware_threads == 0) hardware_threads = 4;

        int num_threads = std::min(hardware_threads, user_thread_limit);
        if (num_threads < 1) num_threads = 1;

        std::vector<std::thread> threads;
        int rows_per_thread = height / num_threads;

        for (int i = 0; i < num_threads; ++i) {
            int start_y = i * rows_per_thread;
            int end_y = (i == num_threads - 1) ? height : start_y + rows_per_thread;
            threads.emplace_back(
                render_band,
                start_y, end_y, width, height,
                pixel_buffer, std::ref(interpreter), std::ref(stack), view_state, std::ref(rows_completed)
            );
        }

        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }

        is_rendering = false;

        }).detach();
}