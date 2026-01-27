from manim import *

class FunctionTable(Scene):
    def construct(self):
        title = MathTex(r"f: \mathbb{R} \rightarrow \mathbb{R}")
        func_def = MathTex(r"f(x) = x^2")
        
        title.to_edge(UP, buff=1.0)
        func_def.next_to(title, DOWN)
        
        self.play(Write(title))
        self.play(Write(func_def))
        self.wait(1)

        # ERASE INTRO
        self.play(
            FadeOut(title), 
            FadeOut(func_def)
        )
        v_line_start = UP * 3
        v_line_end = DOWN * 3
        

        h_line_y = 2.0  

        v_line = Line(v_line_start, v_line_end)
        
        h_line = Line(LEFT * 4, RIGHT * 4).move_to([0, h_line_y, 0])
        
        header_y = h_line_y + 0.6
        col_offset = 2.0
        
        h_x = MathTex("x").scale(1.2).move_to([-col_offset, header_y, 0])
        h_fx = MathTex("f(x)").scale(1.2).move_to([col_offset, header_y, 0])
        
        self.play(
            Create(v_line),
            Create(h_line),
            Write(h_x),
            Write(h_fx)
        )
        
        x_vals = [-3, -2, -1, 0, 1, 2]
        rows = []
        
        row_start_y = h_line_y - 0.8 
        row_spacing = 0.8
        
        for i, x in enumerate(x_vals):
            fx = x**2
            
            current_y = row_start_y - (i * row_spacing)
            
            t_x = MathTex(str(x))
            t_fx = MathTex(str(fx))
            
            t_x.move_to([-col_offset, current_y, 0])
            t_fx.move_to([col_offset, current_y, 0])
            
            rows.append(VGroup(t_x, t_fx))
            
        for row in rows:
            self.play(Write(row), run_time=0.3)
        
        self.wait(2)
        
class ParallelLines(Scene):
    def construct(self):
        ax_top = NumberLine(
            x_range=[-4, 4, 1],
            length=10,
            color=BLUE,
            include_numbers=True,
            label_direction=UP,
        ).shift(UP * 1.5)
        
        ax_bottom = NumberLine(
            x_range=[-1, 10, 1],
            length=10,
            color=RED,
            include_numbers=True,
            label_direction=DOWN,
        ).shift(DOWN * 1.5)

        lab_top = MathTex("x").next_to(ax_top, UP, buff=0.5)
        
        lab_bot = MathTex("f(x) = x^2").next_to(ax_bottom, UP, buff=0.3).add_background_rectangle()

        self.play(Create(ax_top), Write(lab_top))
        self.play(Create(ax_bottom), Write(lab_bot))
        
        x_values = [-3, -2, -1, 0, 1, 2]
        
        for x in x_values:
            val_x = x
            val_y = x**2
            
            dot_top = Dot(ax_top.n2p(val_x), color=BLUE)
            dot_bot = Dot(ax_bottom.n2p(val_y), color=RED)
            
            arrow = Arrow(dot_top.get_center(), dot_bot.get_center(), buff=0.1, color=YELLOW)

            self.play(FadeIn(dot_top), run_time=0.3)
            self.play(GrowArrow(arrow), FadeIn(dot_bot), run_time=0.5)
            self.wait(0.3)
            self.play(FadeOut(arrow), run_time=0.2)
        
        self.wait(2)

class CartesianConstruction(Scene):
    def construct(self):
        # 1. Create Axes
        axes = Axes(
            x_range=[-4, 4, 1],
            y_range=[-1, 10, 1],
            x_length=7,
            y_length=6,
            axis_config={"include_numbers": True},
        )
        
        labels = axes.get_axis_labels(x_label="x", y_label="f(x)")
        
        self.play(Create(axes), Write(labels))
        self.wait(0.5)
        
        x_vals = [-3, -2, -1, 0, 1, 2]
        points = VGroup() 

        for x in x_vals:
            y = x**2
            
            pt_on_x = axes.c2p(x, 0)
            pt_on_y = axes.c2p(0, y)
            final_pt = axes.c2p(x, y)
            
            dot_x = Dot(pt_on_x, color=BLUE, radius=0.08)
            dot_y = Dot(pt_on_y, color=RED, radius=0.08)
            
            line_vert = DashedLine(pt_on_x, final_pt, color=BLUE_E)
            line_horiz = DashedLine(pt_on_y, final_pt, color=RED_E)
            
            dot_final = Dot(final_pt, color=YELLOW)
            points.add(dot_final)

            self.play(FadeIn(dot_x), FadeIn(dot_y), run_time=0.3)
            self.play(Create(line_vert), Create(line_horiz), run_time=0.5)
            self.play(FadeIn(dot_final), run_time=0.2)
            
            self.play(FadeOut(dot_x), FadeOut(dot_y), FadeOut(line_vert), FadeOut(line_horiz), run_time=0.2)
        
        self.wait(1)

        graph = axes.plot(lambda x: x**2, color=YELLOW)
        
        
        self.play(Create(graph), run_time=3)
        
        self.play(graph.animate.set_stroke(width=6))
        self.wait(2)

from manim import *

class ParaboloidPlot(ThreeDScene):
    def construct(self):
        axes = ThreeDAxes(
            x_range=[-3, 3, 1],
            y_range=[-3, 3, 1],
            z_range=[-1, 9, 1],
            x_length=6,
            y_length=6,
            z_length=5
        )
        
        labels = axes.get_axis_labels(
            x_label="x", 
            y_label="y", 
            z_label="g(x,y)"
        )

        surface = Surface(
            lambda u, v: axes.c2p(u, v, u**2 + v**2),
            u_range=[-2.5, 2.5],
            v_range=[-2.5, 2.5],
            resolution=(30, 30),
        )

        surface.set_style(fill_opacity=0.6, stroke_color=BLUE)
        surface.set_fill_by_checkerboard(BLUE, BLUE_E, opacity=0.6)

        func_text = MathTex(r"g(x,y) = x^2 + y^2")
        func_text.to_corner(UL) 

        self.set_camera_orientation(phi=60 * DEGREES, theta=-45 * DEGREES)

        self.add(axes, labels)

        self.add_fixed_in_frame_mobjects(func_text)

        self.play(Create(surface), run_time=2)
        
        self.begin_ambient_camera_rotation(rate=0.2)
        self.wait(9)
        self.stop_ambient_camera_rotation()

class FourDimensionProblem(ThreeDScene):
    def construct(self):
        self.set_camera_orientation(phi=75 * DEGREES, theta=-45 * DEGREES)

        ax_x = Arrow3D(start=ORIGIN, end=RIGHT * 4, color=BLUE)
        lab_x = MathTex(r"\text{Re}(z)").next_to(ax_x.get_end(), RIGHT).rotate(PI/2, axis=RIGHT)
        
        ax_y = Arrow3D(start=ORIGIN, end=UP * 4, color=BLUE)
        lab_y = MathTex(r"\text{Im}(z)").next_to(ax_y.get_end(), UP).rotate(PI/2, axis=RIGHT)
        
        ax_z = Arrow3D(start=ORIGIN, end=OUT * 4, color=RED)
        lab_z = MathTex(r"\text{Re}(f(z))").next_to(ax_z.get_end(), OUT).rotate(PI/2, axis=RIGHT)
        
        ax_w = Arrow3D(start=ORIGIN, end=np.array([2, 2, 2]), color=RED) 
        lab_w = MathTex(r"\text{Im}(f(z))").next_to(ax_w.get_end(), UP).rotate(PI/2, axis=RIGHT)
        
        text_1 = Text("1. Real Input", font_size=24).to_corner(UL)
        text_2 = Text("2. Imaginary Input", font_size=24).to_corner(UL)
        text_3 = Text("3. Real Output", font_size=24).to_corner(UL)
        text_4 = Text("4. Imaginary Output...?", font_size=24, color=RED).to_corner(UL)
        
        self.add_fixed_in_frame_mobjects(text_1)

        self.play(Create(ax_x), Write(lab_x))
        self.wait(0.5)

        self.remove(text_1)
        self.add_fixed_in_frame_mobjects(text_2)
        self.play(Create(ax_y), Write(lab_y))
        self.wait(0.5)

        self.remove(text_2)
        self.add_fixed_in_frame_mobjects(text_3)
        self.play(Create(ax_z), Write(lab_z))
        self.wait(1)

        self.remove(text_3)
        self.add_fixed_in_frame_mobjects(text_4)
        
        self.play(Create(ax_w), Write(lab_w))
        
        w_group = VGroup(ax_w, lab_w)
        
        self.play(Rotate(w_group, angle=PI, axis=UP), run_time=1)
        self.wait(0.3)
        
        self.play(Rotate(w_group, angle=PI/2, axis=RIGHT), run_time=1)
        self.wait(0.3)
        
        self.play(Rotate(w_group, angle=2*PI, axis=OUT), run_time=2)
        
        failure_text = MathTex(r"\text{No Space!}", color=RED).move_to(ax_w.get_end())
        self.add_fixed_in_frame_mobjects(failure_text) 
        
        self.play(
            Wiggle(w_group), 
            FadeIn(failure_text, shift=UP)
        )
        self.wait(1)
        
        self.play(FadeOut(w_group), FadeOut(failure_text))
        self.wait(1)