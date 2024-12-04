set MAX_LINES 14
set MAX_COLS 49
set VIEWPORT_VERTICAL_OFFSET 0
set VIEWPORT_HORIZONTAL_OFFSET 0

set BUFFER [concat [list "" "" "" "" "" "" "" "" "" "" "" "" "" ""]]
set searchpattern ""
set cursorPosition [list 0 0]
set previousCursorPosition [list 0 0]

set monoFont {Courier 12}

wm title . "s"

frame .content
pack .content -side top -fill both -expand true

canvas .content.canvas -width 500 -height 300
pack .content.canvas -side left -fill both -expand true

proc update_line {row} {
    global BUFFER cursorPosition monoFont VIEWPORT_HORIZONTAL_OFFSET MAX_COLS
    set line [lindex $BUFFER $row]
    .content.canvas delete "line$row"

    set visible_line [string range $line $VIEWPORT_HORIZONTAL_OFFSET [expr {$VIEWPORT_HORIZONTAL_OFFSET + $MAX_COLS - 1}]]

    .content.canvas create text 10 [expr {20 * $row}] -anchor nw -font $monoFont -text $visible_line -tags "line$row"

    if {$row == [lindex $cursorPosition 0]} {
        draw_cursor_box
    }
}

set command_tab_text ""
set commandStatusIndicator "\[ \]"

proc draw_command_tab {} {
    global monoFont MAX_COLS command_tab_text commandStatusIndicator

    set canvasWidth [winfo width .content.canvas]
    set canvasHeight [winfo height .content.canvas]

    set lineHeight 20
    set rectY1 [expr {$canvasHeight - $lineHeight}]
    set rectY2 $canvasHeight

    .content.canvas delete command_tab
    .content.canvas delete command_tab_t

    .content.canvas create rectangle 0 $rectY1 $canvasWidth $rectY2 -fill lightgray -outline black -tags command_tab

    set charWidth [font measure $monoFont " "]
    set textX 10
    set textY [expr {$rectY1 + ($lineHeight / 4)}]
    
    set displayText "$commandStatusIndicator > $command_tab_text"

    .content.canvas create text $textX $textY -anchor nw -font $monoFont -text $displayText -tags command_tab_t
}


proc update_display {} {
    global BUFFER monoFont command_tab_open
    .content.canvas delete all

    set row 0
    foreach line $BUFFER {
        update_line $row
        incr row
    }

    if {$command_tab_open == 1} {
        # bleh
        .content.canvas delete selected_box
        draw_command_tab
    } else {
        draw_cursor_box
    }
}

proc draw_cursor_box {} {
    global BUFFER monoFont cursorPosition VIEWPORT_HORIZONTAL_OFFSET

    .content.canvas delete selected_box

    set row [lindex $cursorPosition 0]
    set col [lindex $cursorPosition 1]

    set line [lindex $BUFFER $row]

    set char_width [font measure $monoFont " "]

    set x1 [expr {10 + ($col - $VIEWPORT_HORIZONTAL_OFFSET) * $char_width}]
    set x2 [expr {$x1 + $char_width}]

    set y1 [expr {20 * $row}]
    set y2 [expr {$y1 + 20}]

    .content.canvas create rectangle $x1 $y1 $x2 $y2 -outline black -width 2 -tags selected_box
}

proc move_cursor {direction} {
    global BUFFER cursorPosition previousCursorPosition VIEWPORT_VERTICAL_OFFSET VIEWPORT_HORIZONTAL_OFFSET MAX_LINES MAX_COLS
    set previousCursorPosition $cursorPosition

    switch -- $direction {
        "up" {
            if {$VIEWPORT_VERTICAL_OFFSET > 0 && [lindex $cursorPosition 0] == 0} {
                set VIEWPORT_VERTICAL_OFFSET [expr {$VIEWPORT_VERTICAL_OFFSET - 1}]
                networking::send "viewport 0 $MAX_LINES $VIEWPORT_VERTICAL_OFFSET"
            }

            if {[lindex $cursorPosition 0] > 0} {
                set cursorPosition [list [expr {[lindex $cursorPosition 0] - 1}] [lindex $cursorPosition 1]]
            }

            set lineLen [string length [lindex $BUFFER [lindex $cursorPosition 0]]]
            if {[lindex $cursorPosition 1] > $lineLen} {
                set cursorPosition [list [lindex $cursorPosition 0] $lineLen]
            }

            if {$lineLen > $MAX_COLS} {
                set VIEWPORT_HORIZONTAL_OFFSET [expr {$lineLen - $MAX_COLS}]
            } else {
                set VIEWPORT_HORIZONTAL_OFFSET 0
            }
        }
        "down" {
            if {[lindex $cursorPosition 0] == [expr {$MAX_LINES - 1}]} {
                set VIEWPORT_VERTICAL_OFFSET [expr {$VIEWPORT_VERTICAL_OFFSET + 1}]
                networking::send "viewport 0 $MAX_LINES $VIEWPORT_VERTICAL_OFFSET"
            }

            if {[lindex $cursorPosition 0] < [expr {[llength $BUFFER] - 1}]} {
                set cursorPosition [list [expr {[lindex $cursorPosition 0] + 1}] [lindex $cursorPosition 1]]
            }

            set lineLen [string length [lindex $BUFFER [lindex $cursorPosition 0]]]
            if {[lindex $cursorPosition 1] > $lineLen} {
                set cursorPosition [list [lindex $cursorPosition 0] $lineLen]
            }

            if {$lineLen > $MAX_COLS} {
                set VIEWPORT_HORIZONTAL_OFFSET [expr {$lineLen - $MAX_COLS}]
            } else {
                set VIEWPORT_HORIZONTAL_OFFSET 0
            }
        }
        "left" {
            if {[lindex $cursorPosition 1] > 0} {
                # move left within the current line
                set cursorPosition [list [lindex $cursorPosition 0] [expr {[lindex $cursorPosition 1] - 1}]]

                if {[lindex $cursorPosition 1] < $VIEWPORT_HORIZONTAL_OFFSET} {
                    set VIEWPORT_HORIZONTAL_OFFSET [expr {$VIEWPORT_HORIZONTAL_OFFSET - 1}]
                    
                    if {$VIEWPORT_HORIZONTAL_OFFSET < 0} {
                        set VIEWPORT_HORIZONTAL_OFFSET 0
                    }
                }
            } elseif {[lindex $cursorPosition 0] > 0} {
                # move to the end of the previous line
                set prevLine [expr {[lindex $cursorPosition 0] - 1}]
                set lineLen [string length [lindex $BUFFER $prevLine]]
                set cursorPosition [list $prevLine $lineLen]

                if {$lineLen > $MAX_COLS} {
                    set VIEWPORT_HORIZONTAL_OFFSET [expr {$lineLen - $MAX_COLS}]
                } else {
                    set VIEWPORT_HORIZONTAL_OFFSET 0
                }
            }
        }
        "right" {
            set lineLen [string length [lindex $BUFFER [lindex $cursorPosition 0]]]
            if {[lindex $cursorPosition 1] < $lineLen} {
                # move right within the current line
                set cursorPosition [list [lindex $cursorPosition 0] [expr {[lindex $cursorPosition 1] + 1}]]

                if {[lindex $cursorPosition 1] >= [expr {$VIEWPORT_HORIZONTAL_OFFSET + $MAX_COLS}]} {
                    set VIEWPORT_HORIZONTAL_OFFSET [expr {$VIEWPORT_HORIZONTAL_OFFSET + 1}]
                }
            } elseif {[lindex $cursorPosition 0] < [expr {[llength $BUFFER] - 1}]} {
                # move to the start of the next line
                set cursorPosition [list [expr {[lindex $cursorPosition 0] + 1}] 0]
                set VIEWPORT_HORIZONTAL_OFFSET 0
            }
        }
        default {
            puts "invalid direction"
        }
    }
    
    # this used to update only changes now all of the lines, might change it back
    update_display
}

proc jump_word {direction} {
    global BUFFER cursorPosition

    if {$direction eq "left"} {
        set move_cmd "left"
        set adjust_needed true
    } elseif {$direction eq "right"} {
        set move_cmd "right"
        set adjust_needed false
    } else {
        puts "invalid direction for jump_word: $direction"
        return
    }

    if {[initial_move $move_cmd]} {
        return
    }

    skip_whitespace $move_cmd

    move_across_word $move_cmd

    if {$adjust_needed} {
        adjust_cursor_to_word_start
    }
}

proc initial_move {move_cmd} {
    global cursorPosition

    set prevPos $cursorPosition
    move_cursor $move_cmd
    return [expr {$cursorPosition eq $prevPos}]
}

proc skip_whitespace {move_cmd} {
    global BUFFER cursorPosition

    update_position_vars
    while {[within_bounds $move_cmd] && [string is space [string index $::line $::col]]} {
        if {[initial_move $move_cmd]} { return }

        update_position_vars
    }
}

proc move_across_word {move_cmd} {
    global BUFFER cursorPosition

    update_position_vars
    while {[within_bounds $move_cmd] && ![string is space [string index $::line $::col]]} {
        if {[initial_move $move_cmd]} { break }

        update_position_vars
    }
}

proc update_position_vars {} {
    global BUFFER cursorPosition line col row lineLen
    set row [lindex $cursorPosition 0]
    set col [lindex $cursorPosition 1]
    set line [lindex $BUFFER $row]
    set lineLen [string length $line]
}

proc within_bounds {move_cmd} {
    if {$move_cmd eq "left"} {
        return [expr {$::col >= 0}]
    } else {
        return [expr {$::col < $::lineLen}]
    }
}

proc adjust_cursor_to_word_start {} {
    global line col
    if {$col < 0 || [string is space [string index $line $col]]} {
        move_cursor right
    }
}

array set key_translation_table {
    space " " 
    exclam "!" 
    at "@" 
    numbersign "#" 
    dollar "$" 
    percent "%" 
    asciicircum "^" 
    ampersand "&" 
    asterisk "*" 
    parenleft "(" 
    parenright ")" 
    minus "-" 
    underscore "_" 
    plus "+" 
    equal "=" 
    bracketleft "[" 
    bracketright "]" 
    braceleft "{" 
    braceright "}" 
    semicolon ";" 
    colon ":" 
    quote "'" 
    doublequote "\"" 
    backslash "\\" 
    slash "/" 
    comma "," 
    period "." 
    less "<" 
    greater ">" 
    question "?" 
    bar "|" 
    grave "`" 
    asciitilde "~"
}

proc command_tab_handle_input {key} {
    global command_tab_text command_tab_open key_translation_table commandStatusIndicator
    if { $key eq "BackSpace" } {
        if {[string length $command_tab_text] > 0} {
            set command_tab_text [string range $command_tab_text 0 end-1]
        }
    } elseif {$key eq "Return"} {
        networking::send $command_tab_text
        set commandStatusIndicator "\[?\]"
    } elseif {[info exists key_translation_table($key)]} {
        append command_tab_text $key_translation_table($key)
    } elseif {[string is alnum $key]} {
        append command_tab_text $key
    } else {
        #ignore everything else
    }

    update_display
}

proc dispatch_user_input {command} {
    global command_tab_open
    if { $command_tab_open == 1 } {
        # command_tab_handle_input $command
    } else {
        eval $command
    }
}

bind . <Key-Up>         { dispatch_user_input "move_cursor up"   }
bind . <Key-Down>       { dispatch_user_input "move_cursor down" }
bind . <Key-Left>       { dispatch_user_input "move_cursor left" }
bind . <Key-Right>      { dispatch_user_input "move_cursor right"}
bind . <Control-Left>   { dispatch_user_input "jump_word left"   }
bind . <Control-Right>  { dispatch_user_input "jump_word right"  }

bind . <KeyPress>       { handle_key_press %K                    }

set queue {}
set times {}

proc handle_key_press {k} {
    global cursorPosition queue times command_tab_open
    
    if {$queue eq "Control_L" && $k eq "c"} {
        if {[expr [clock milliseconds] - $times] <= 500} {
            commands::toggle_command_tab
            set queue {}; set times {}
            return
        }
    }
    set queue $k
    set times [clock milliseconds]

    if { $command_tab_open == 1 } {
        command_tab_handle_input $k
    } else {    
        set lin [lindex $cursorPosition 1]
        set col [lindex $cursorPosition 0]
        networking::send "key $col $lin $k"
    }
}

proc replace_line {lineNumber newContent} {
    global BUFFER
    if {$lineNumber < 0 || $lineNumber >= [llength $BUFFER]} {
        puts "line number $lineNumber is out of range"
        return
    }

    set BUFFER [lreplace $BUFFER $lineNumber $lineNumber $newContent]

    update_line $lineNumber
}

proc empty_line {lineNumber} {
    global BUFFER
    if {$lineNumber < 0 || $lineNumber >= [llength $BUFFER]} {
        puts "line number $lineNumber is out of range"
        return
    }

    set BUFFER [lreplace $BUFFER $lineNumber $lineNumber ""]

    update_line $lineNumber
}

proc set_cursor_pos {line col} {
    global cursorPosition
    set cursorPosition [lset cursorPosition 0 [expr {$line + 0}]]
    set cursorPosition [lset cursorPosition 1 [expr {$col + 0}]]
}

proc update_viewport {} {
    global BUFFER
    set len [llength $BUFFER]
    networking::send "viewport 0 $len"
}