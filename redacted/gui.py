import sys

from rich.syntax import Syntax
from rich.traceback import Traceback

from textual import events, reactive
from textual.app import App, ComposeResult
from textual.containers import Container, Vertical
from textual.reactive import var
from textual.widget import Widget, MountError
from textual.widgets import DirectoryTree, Footer, Header, Static, Button, Label

from datastructs import *


class ListButtons(Widget):
    def compose(self) -> ComposeResult:
        for i in range(4):
            yield Button("-", id="slave-btn-"+str(i))

class SlavesManager(App):
    """Textual code browser app."""

    TITLE = "[REDACTED]"
    SUB_TITLE = "Manage slave devices"

    CSS_PATH = "slaves_manager.css"
    BINDINGS = [
        ("f", "toggle_files", "Toggle Files"),
        ("q", "quit", "Quit"),
    ]

    show_tree = var(True)

    slaves = { }

    slaves_btn = []

    def watch_show_tree(self, show_tree: bool) -> None:
        """Called when show_tree is modified."""
        self.set_class(show_tree, "-show-tree")

    msg_label = Label("Do you love Textual?", id="msg-lbl")
    slavbtn = Button("Slave -", id="slave-btn-0")

    list_buttons = Vertical(
            #Button("slave ---"),
            Button("slave ---"),
            Button("slave ---"),
            id="list-buttons"
        )

    def compose(self) -> ComposeResult:
        """Compose our UI."""
        path = "./" if len(sys.argv) < 2 else sys.argv[1]
        yield Header()
        
        yield ListButtons()

        #yield self.list_buttons
        
        yield Vertical(
            self.msg_label,
            id="code-view")
        
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

    def on_button_pressed(self, event: Button.Pressed):

        index = int(event.button.id.split("-")[-1])
        if index >= len(self.slaves_btn):
            self.change_label_test(str(index) + ">" + str(len(self.slaves_btn)))
            return
        
        slave = self.slaves[self.slaves_btn[index]]
        s = ''
        s += 'name: {}\n'.format(slave.name)
        s += '\tsensors:\n'
        for i in slave.sensors:
            s += '\tname: {}\n'.format(i.name)
            s += '\t\tvalue {}\n'.format(i.val)
        #print(s.strip())

        self.change_label_test(s)

    def action_toggle_files(self) -> None:
        """Called in response to key binding."""
        self.show_tree = not self.show_tree

    def change_label_test(self, msg):
        self.msg_label.update(renderable=str(msg))

    

    ### MESSAGE PARSER ###


    def send_msg(self, msg):

        slave_msg = msg["msg"]
        
        if slave_msg["type"] == 0:
            self.slaves[msg["from"]] = Slave(msg["from"], slave_msg["name"])

        elif slave_msg["type"] == 2:
            if msg["from"] in self.slaves:
                self.slaves[msg["from"]].set_sensors(slave_msg["sensors"])
                self.add_slave(msg["from"], slave_msg)
                print("\n\n\n\n\n\n\n\n")
                for s in self.slaves:
                    print(s)
        
        elif slave_msg["type"] == 9:
            if msg["from"] in self.slaves:
                for s in slave_msg["sensors"]:
                    self.slaves[msg["from"]].sensors[s["index"]].val = s["val"]
                print("\n\n\n\n\n\n\n\n")
                # for k in slaves:
                #     slave = slaves[k]
                #     s = ''
                #     s += 'name: {}\n'.format(slave.name)
                #     s += '\tsensors:\n'
                #     for i in slave.sensors:
                #         s += '\tname: {}\n'.format(i.name)
                #         s += '\t\tvalue {}\n'.format(i.val)
                #     print(s.strip())

    def add_slave(self, addr, s):
        #self.change_label_test(addr)
        self.query_one("#msg-lbl").update(renderable=str(addr))
        self.query_one("#slave-btn-"+str(len(self.slaves_btn))).label = "Slave " + str(addr)
        self.slaves_btn.append(addr)

    # def on_key(self, event: events.Key):
    #     self.query_one("#slave-btn-2").label = "aaa"
