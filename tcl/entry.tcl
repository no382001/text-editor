package require logger 0.3
set log [logger::init tcl]

source tcl/commands.tcl
source tcl/networking.tcl
source tcl/editor.tcl

networking::connect
update_display
update_viewport