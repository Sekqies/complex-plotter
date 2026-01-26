from manim import *

class CleanSlate(Scene):
    def construct(self):
        dot = Dot(color=YELLOW)
        label = Text("It's alive!", font_size=24).next_to(dot, UP)
        
        self.play(FadeIn(dot, scale=3), Write(label))
        self.wait(2)