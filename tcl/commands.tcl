package require base64

namespace eval commands {
     proc eval_command {string} {
        global log
        global cursorPosition

        set parts [split $string " "]
        set command [lindex $parts 0]
    
        ${log}::notice "$string"

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
            default {
                puts "unknown command: $command"
            }
        }

        update_line [lindex $cursorPosition 0]
    }
}