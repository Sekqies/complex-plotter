from manim import *
import colorsys

class ComplexDefinition(Scene):
    def construct(self):
        eq = MathTex(
            "z", "=", "a", "+", "b", "i",
            font_size=120
        )
        eq.set_color_by_tex("a", BLUE)
        eq.set_color_by_tex("b", GREEN)
        eq.set_color_by_tex("i", YELLOW)
        
        self.play(Write(eq))
        self.wait(1)
        
        target_a = eq.get_part_by_tex("a").get_top()
        start_a = target_a + UP * 2 + LEFT * 0.5
        
        arrow_a = Arrow(start=start_a, end=target_a, color=BLUE)
        label_a = Text("Real Component", font_size=32, color=BLUE).next_to(arrow_a.get_start(), UP)
        
        self.play(GrowArrow(arrow_a), Write(label_a))
        self.wait(1) 

        target_b = eq.get_part_by_tex("b").get_bottom()
        start_b = target_b + DOWN * 2
        
        arrow_b = Arrow(start=start_b, end=target_b, color=GREEN)
        label_b = Text("Imaginary Component", font_size=32, color=GREEN).next_to(arrow_b.get_start(), DOWN)
        
        self.play(GrowArrow(arrow_b), Write(label_b))
        self.wait(1)

        target_i = eq.get_part_by_tex("i").get_right()
        start_i = target_i + RIGHT * 2 + UP * 0.5
        
        arrow_i = Arrow(start=start_i, end=target_i, color=YELLOW)
        label_i = Text("Imaginary Unit", font_size=32, color=YELLOW).next_to(arrow_i.get_start(), UP)
        
        self.play(GrowArrow(arrow_i), Write(label_i))
        self.wait(2)


class ComplexMultiplication(Scene):
    def construct(self):
        title = Text("Multiplication Logic", font_size=36).to_edge(UP)
        self.add(title)

        step1 = MathTex(r"(a + bi) \cdot (c + di)", font_size=48)
        self.play(Write(step1))
        self.wait(1)
        
        step2 = MathTex(r"= ac + adi + bci + bdi^2", font_size=48)
        self.play(TransformMatchingTex(step1, step2))
        self.wait(1)
        
        step2_colored = MathTex(r"= ac + adi + bci + bd", r"i^2", font_size=48)
        step2_colored.set_color_by_tex("i^2", YELLOW)
        self.remove(step2)
        self.add(step2_colored)
        self.wait(0.5)
        
        step3 = MathTex(r"= ac + (ad + bc)i + bd(-1)", font_size=48)
        self.play(TransformMatchingTex(step2_colored, step3))
        self.wait(1)
        
        step4 = MathTex(r"= (ac - bd) + (ad + bc)i", font_size=48)
        step4[0][2:7].set_color(BLUE)  
        step4[0][9:15].set_color(GREEN) 
        
        self.play(TransformMatchingTex(step3, step4))
        
        box = SurroundingRectangle(step4, color=YELLOW, buff=0.2)
        self.play(Create(box))
        self.wait(2)


class ComplexPlotting(Scene):
    def construct(self):

        plane = ComplexPlane(
            x_range=[-5, 5, 1],
            y_range=[-4, 4, 1],
            background_line_style={"stroke_opacity": 0.5}
        ).add_coordinates()
        
        labels = plane.get_axis_labels(x_label="Re(z)", y_label="Im(z)")
        
        self.play(Create(plane), Write(labels))

        points_data = [
            (3 + 2j, "3 + 2i", BLUE),
            (-2 + 1j, "-2 + i", GREEN),
            (2 - 3j, "2 - 3i", RED),
            (-4 - 2j, "-4 - 2i", ORANGE)
        ]
        
        for z_val, label_tex, col in points_data:
            dot = Dot(plane.n2p(z_val), color=col)
            line = Line(plane.n2p(0), plane.n2p(z_val), color=col, stroke_opacity=0.5)
            
            lbl = MathTex(label_tex, color=col).next_to(dot, UR, buff=0.1)
            lbl.add_background_rectangle()
            
            self.play(GrowFromCenter(dot), Create(line), run_time=0.5)
            self.play(Write(lbl), run_time=0.5)
            self.wait(0.5)
            
        self.wait(2)


class HSLMapping(Scene):
    def construct(self):
        res = 400
        image_array = np.zeros((res, res, 4), dtype=np.uint8)
        center = res / 2
        radius_px = res / 2
        
        for y in range(res):
            for x in range(res):
                dx = (x - center) / radius_px
                dy = (y - center) / radius_px
                dist = np.sqrt(dx*dx + dy*dy)
                
                if dist <= 1.0:
                    angle = np.arctan2(-dy, dx)
                    if angle < 0: angle += TAU
                    hue = angle / TAU
                    rgb = colorsys.hls_to_rgb(hue, dist * 0.5, 1.0)
                    
                    image_array[y, x, 0] = int(rgb[0] * 255)
                    image_array[y, x, 1] = int(rgb[1] * 255)
                    image_array[y, x, 2] = int(rgb[2] * 255)
                    image_array[y, x, 3] = 255
                else:
                    image_array[y, x] = [0,0,0,0]

        wheel_img = ImageMobject(image_array).scale(2.5)
        
        rect = RoundedRectangle(corner_radius=0.2, width=3.5, height=3, color=WHITE, fill_color=BLACK, fill_opacity=0.8)
        rect.to_corner(UR)
        
        val_l = DecimalNumber(0, num_decimal_places=2).scale(0.8)
        val_t = DecimalNumber(0, num_decimal_places=0, unit="^\circ").scale(0.8)
        color_preview = Square(side_length=0.4, fill_opacity=1, stroke_color=WHITE, stroke_width=2)
        hex_label = Text("#FFFFFF", font="Monospace", font_size=24)
        
        lbl_group = VGroup(
            VGroup(MathTex("L: "), val_l).arrange(RIGHT),
            VGroup(MathTex(r"\theta: "), val_t).arrange(RIGHT),
            VGroup(Text("Color: ", font_size=24), color_preview).arrange(RIGHT),
            hex_label
        ).arrange(DOWN, aligned_edge=LEFT, buff=0.3).move_to(rect)
        
        self.play(FadeIn(wheel_img), Create(rect), Write(lbl_group))
        
        tracker_t = ValueTracker(0)
        tracker_l = ValueTracker(1)

        line = Line(ORIGIN, RIGHT * 2.5, color=WHITE, stroke_width=4)
        dot = Dot(color=WHITE).move_to(line.get_end())

        def update_line_geometry(mob):
            angle_rad = tracker_t.get_value() * DEGREES
            current_len = tracker_l.get_value() * 2.5 
            
            new_end = np.array([
                current_len * np.cos(angle_rad),
                current_len * np.sin(angle_rad),
                0
            ])
            
            mob.put_start_and_end_on(ORIGIN, new_end)

        line.add_updater(update_line_geometry)
        dot.add_updater(lambda m: m.move_to(line.get_end()))
        
        val_t.add_updater(lambda m: m.set_value(tracker_t.get_value() % 360))
        val_l.add_updater(lambda m: m.set_value(tracker_l.get_value()))
        
        def update_color_info(mob):
            angle_deg = tracker_t.get_value()
            length = tracker_l.get_value()
            
            hue = (angle_deg % 360) / 360.0
            light = length * 0.5 
            
            rgb = colorsys.hls_to_rgb(hue, light, 1.0)
            manim_col = ManimColor(rgb)
            
            color_preview.set_fill(manim_col)
            
            r, g, b = [int(c * 255) for c in rgb]
            hex_str = "#{:02X}{:02X}{:02X}".format(r, g, b)
            
            new_text = Text(hex_str, font="Monospace", font_size=24).move_to(hex_label)
            hex_label.become(new_text)

        hex_label.add_updater(update_color_info)
        
        self.add(line, dot)
        
        self.play(tracker_t.animate.set_value(360), run_time=3, rate_func=linear)
        self.play(tracker_l.animate.set_value(0.5), run_time=1)
        self.play(tracker_l.animate.set_value(1.0), run_time=1)
        self.play(tracker_t.animate.set_value(720), tracker_l.animate.set_value(0.2), run_time=2)

class GridWarping(Scene):
    def construct(self):
        grid = ComplexPlane(
            x_range=[-2, 2, 1],
            y_range=[-2, 2, 1],
            background_line_style={
                "stroke_color": BLUE_C,
                "stroke_width": 2,
                "stroke_opacity": 0.5
            }
        ).add_coordinates()

        grid.prepare_for_nonlinear_transform()

        func_label = MathTex("f(z) = z").to_corner(UL).add_background_rectangle()
        
        self.add(grid, func_label)
        self.wait(1)

        def complex_square(point):
            x, y, z = point
            c = complex(x, y)
            c_squared = c**2
            return [c_squared.real, c_squared.imag, 0]

        new_label = MathTex("f(z) = z^2").to_corner(UL).add_background_rectangle()
        
        self.play(ReplacementTransform(func_label, new_label))
        
        self.play(
            ApplyPointwiseFunction(complex_square, grid),
            run_time=3,
            rate_func=linear
        )
        
        self.wait(2)