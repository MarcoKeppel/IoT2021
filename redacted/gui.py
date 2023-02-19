import sys

from rich.syntax import Syntax
from rich.traceback import Traceback

from textual import events
from textual.app import App, ComposeResult
from textual.containers import Container, Vertical
from textual.reactive import var
from textual.widget import Widget
from textual.widgets import DirectoryTree, Footer, Header, Static, Button


class ListButtons(Widget):
    def compose(self) -> ComposeResult:
        for i in range(4):
            yield Button("AAC"+str(i), id="Aac"+str(i), variant="primary")

class SlavesManager(App):
    """Textual code browser app."""

    CSS_PATH = "slaves_manager.css"
    BINDINGS = [
        ("f", "toggle_files", "Toggle Files"),
        ("q", "quit", "Quit"),
    ]

    show_tree = var(True)

    def watch_show_tree(self, show_tree: bool) -> None:
        """Called when show_tree is modified."""
        self.set_class(show_tree, "-show-tree")

    def compose(self) -> ComposeResult:
        """Compose our UI."""
        path = "./" if len(sys.argv) < 2 else sys.argv[1]
        yield Header()
        
        yield ListButtons()
        
        yield Vertical(Button("ASC", id="asc", variant="primary"), id="code-view")
        yield Footer()

    def on_mount(self, event: events.Mount) -> None:
        #self.query_one(DirectoryTree).focus()
        pass

    def on_directory_tree_file_selected(
        self, event: DirectoryTree.FileSelected
    ) -> None:
        """Called when the user click a file in the directory tree."""
        event.stop()
        code_view = self.query_one("#code", Static)
        try:
            syntax = Syntax.from_path(
                event.path,
                line_numbers=True,
                word_wrap=False,
                indent_guides=True,
                theme="github-dark",
            )
        except Exception:
            code_view.update(Traceback(theme="github-dark", width=None))
            self.sub_title = "ERROR"
        else:
            code_view.update(syntax)
            self.query_one("#code-view").scroll_home(animate=False)
            self.sub_title = event.path

    def action_toggle_files(self) -> None:
        """Called in response to key binding."""
        self.show_tree = not self.show_tree