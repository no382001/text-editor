namespace eval networking {

    variable server_address "localhost"
    variable server_port 5000
    variable sock ""
    
    # ----------------------------
    # connecting
    # ----------------------------
    proc try_connect {address port} {
        variable sock
        global log

        while {1} {
            if { [catch {socket $address $port} sock] } {
                ${log}::notice "Failed to connect to $address:$port, retrying in 2 seconds..."
                after 2000
            } else {
                ${log}::notice "Connected to $address:$port!"
                break
            }
        }
        return $sock
    }

    proc connect {} {
        variable server_address
        variable server_port
        variable sock
        global log

        set sock [try_connect $server_address $server_port]

        if {$sock ne ""} {
            fconfigure $sock -blocking 0
            ${log}::notice "connection successful, socket configured as non-blocking."
        } else {
            ${log}::error "Failed to connect."
        }

        fileevent $sock readable networking::receive_data
    }

    # ----------------------------
    # sending key press/release
    # ----------------------------
    proc send {string} {
        variable sock
        global log
        if {$sock ne ""} {
            puts $string
            puts -nonewline $sock $string
            flush $sock
        } else {
            ${log}::error "socket is not available."
        }
    }


    # ----------------------------
    # receiving data
    # ----------------------------
    proc receive_data {} {
        variable sock
        global log
        if {[eof $sock]} {
            close $sock
            ${log}::notice "connection closed by server."
            networking::connect
        }

        set data [read $sock]
        commands::eval_command $data
    }
}
