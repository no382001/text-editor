package require base64

namespace eval commands {
     proc eval_command {string} {
        global log
        global cursorPosition

        set parts [split $string " "]
        set command [lindex $parts 0]
        set line [lindex $parts 1]
        set column [lindex $parts 2]
        set data [base64::decode [lindex $parts 3]]

        ${log}::notice "$command $line $column $data"

        switch -- $command {
            "ch" {
                replace_line $line $data
            }
            "pos" {
                set_cursor_pos $line $column
            }
            "el" {
                empty_line $line
            }
            "term" {
                exit 0
            }
            default {
                puts "unknown command: $command"
            }
        }

        update_line [lindex $cursorPosition 0]
    }
}