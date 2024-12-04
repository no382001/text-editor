package require base64

set command_tab_open 0

namespace eval commands {
     proc eval_command {string} {
        global log
        global cursorPosition
        global commandStatusIndicator

        set parts [split $string " "]
        set command [lindex $parts 0]
    
        ${log}::notice "command recieved: '$string' "

        switch -- $command {
            "ch" {
                set line [lindex $parts 1]
                set data [base64::decode [lindex $parts 2]]
                replace_line $line $data
            }
            "pos" {
                set line [lindex $parts 1]
                set column [lindex $parts 2]
                set_cursor_pos $line $column
            }
            "el" {
                set line [lindex $parts 1]
                empty_line $line
            }
            "term" {
                exit 0
            }
            "move" {
                set dir [lindex $parts 1]
                move_cursor $dir
            }
            "cmdack" {
                set commandStatusIndicator "\[x\]"
                update_display
                return
            }
            "cmdnack" {
                set commandStatusIndicator "\[-\]"
                update_display
                return
            }
            default {
                puts "unknown command: $command"
            }
        }

        update_line [lindex $cursorPosition 0]
    }

    proc toggle_command_tab {} {   
        global command_tab_open
        
        if {$command_tab_open == 0} {
            set command_tab_open 1
        } else {
            set command_tab_open 0
        }

        update_display
    }   
}