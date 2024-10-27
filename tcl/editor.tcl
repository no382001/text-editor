package provide editor 0.1

set MAX_LINES 14
set VIEWPORT_OFFSET 0

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
    global BUFFER cursorPosition monoFont
    set line [lindex $BUFFER $row]
    .content.canvas delete "line$row"

    .content.canvas create text 10 [expr {20 * $row}] -anchor nw -font $monoFont -text $line -tags "line$row"

    if {$row == [lindex $cursorPosition 0]} {
        draw_cursor_box
    }
}

proc update_display {} {
    global BUFFER monoFont
    .content.canvas delete all

    set row 0
    foreach line $BUFFER {
        .content.canvas create text 10 [expr {20 * $row}] -anchor nw -font $monoFont -text $line -tags "line$row"
        incr row
    }

    draw_cursor_box
}

proc draw_cursor_box {} {
    global BUFFER monoFont cursorPosition

    .content.canvas delete selected_box

    set row [lindex $cursorPosition 0]
    set col [lindex $cursorPosition 1]

    set line [lindex $BUFFER $row]

    set char_width [font measure $monoFont " "]

    set x1 [expr {10 + $col * $char_width}]
    set x2 [expr {$x1 + $char_width}]

    set y1 [expr {20 * $row}]
    set y2 [expr {$y1 + 20}]

    .content.canvas create rectangle $x1 $y1 $x2 $y2 -outline black -width 2 -tags selected_box
}

proc move_cursor {direction} {
    global BUFFER cursorPosition previousCursorPosition VIEWPORT_OFFSET MAX_LINES
    set previousCursorPosition $cursorPosition

    switch -- $direction {
        "up" {
            if {$VIEWPORT_OFFSET > 0 && [lindex $cursorPosition 0] == 0} {
                set VIEWPORT_OFFSET [expr {$VIEWPORT_OFFSET - 1}]
                networking::send "viewport 0 $MAX_LINES $VIEWPORT_OFFSET"
            }

            if {[lindex $cursorPosition 0] > 0} {
                set cursorPosition [list [expr {[lindex $cursorPosition 0] - 1}] [lindex $cursorPosition 1]]
            }

            set lineLen [string length [lindex $BUFFER [lindex $cursorPosition 0]]]
            if {[lindex $cursorPosition 1] > $lineLen} {
                set cursorPosition [list [lindex $cursorPosition 0] $lineLen]
            }
        }
        "down" {
            if {[lindex $cursorPosition 0] == [expr {$MAX_LINES - 1}]} {
                set VIEWPORT_OFFSET [expr {$VIEWPORT_OFFSET + 1}]
                networking::send "viewport 0 $MAX_LINES $VIEWPORT_OFFSET"
            }

            if {[lindex $cursorPosition 0] < [expr {[llength $BUFFER] - 1}]} {
                set cursorPosition [list [expr {[lindex $cursorPosition 0] + 1}] [lindex $cursorPosition 1]]
            }

            set lineLen [string length [lindex $BUFFER [lindex $cursorPosition 0]]]
            if {[lindex $cursorPosition 1] > $lineLen} {
                set cursorPosition [list [lindex $cursorPosition 0] $lineLen]
            }
        }
        "left" {
            if {[lindex $cursorPosition 1] > 0} {
                # move left within the current line
                set cursorPosition [list [lindex $cursorPosition 0] [expr {[lindex $cursorPosition 1] - 1}]]
            } elseif {[lindex $cursorPosition 0] > 0} {
                # move to the end of the previous line
                set prevLine [expr {[lindex $cursorPosition 0] - 1}]
                set lineLen [string length [lindex $BUFFER $prevLine]]
                set cursorPosition [list $prevLine $lineLen]
            }
        }
        "right" {
            set lineLen [string length [lindex $BUFFER [lindex $cursorPosition 0]]]
            if {[lindex $cursorPosition 1] < $lineLen} {
                # move right within the current line
                set cursorPosition [list [lindex $cursorPosition 0] [expr {[lindex $cursorPosition 1] + 1}]]
            } elseif {[lindex $cursorPosition 0] < [expr {[llength $BUFFER] - 1}]} {
                # move to the start of the next line
                set cursorPosition [list [expr {[lindex $cursorPosition 0] + 1}] 0]
            }
        }
        default {
            puts "invalid direction"
        }
    }
    
    # update only changes
    update_line [lindex $previousCursorPosition 0]
    update_line [lindex $cursorPosition 0]
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

bind . <Key-Up> {move_cursor up}
bind . <Key-Down> {move_cursor down}
bind . <Key-Left> {move_cursor left}
bind . <Key-Right> {move_cursor right}
bind . <Control-Left> {jump_word left}
bind . <Control-Right> {jump_word right}

bind . <KeyPress> {handle_key_press %K}


proc handle_key_press {k} {
    global cursorPosition
    set col [lindex $cursorPosition 0]
    set lin [lindex $cursorPosition 1]
    networking::send "key $col $lin $k"
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