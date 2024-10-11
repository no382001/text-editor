package require base64

namespace eval commands {
     proc eval_command {string} {
        set parts [split $string " "]

        set command [lindex $parts 0]
        set line [lindex $parts 1]
        set column [lindex $parts 2]
        set data [base64::decode [lindex $parts 3]]

        puts "command: $command"
        puts "line: $line"
        puts "column: $column"
        puts "data: $data"

        switch -- $command {
            "ch" {
                replace_line $line $data
            }
            "pos" {
                set_cursor_pos $line $column
            }
            default {
                puts "unknown command: $command"
            }
        }
    }
}