package main

import (
	"errors"
	"fmt"
	"net/http"
)

const port = 8000

type State struct {
    pc bool
    to_turn_on bool
    to_reset bool
}

// func cors() {
//      if r.Method != "POST" {
//          fmt.Println("not POST or OPTIONS method, abort")
//          http.Error(w, "method not supported", http.StatusBadRequest)
//          return
//      }
//
//     if r.Method == "OPTIONS" {
//         fmt.Println("OPTIONS request received")
//         w.Header().Add("Connection", "keep-alive")
//         w.Header().Add("Access-Control-Allow-Origin", "*")
//         w.Header().Add("Access-Control-Allow-Methods", "POST")
//         w.Header().Add("Access-Control-Allow-Headers", "content-type")
//         w.Header().Add("Access-Control-Max-Age", "86400")
//         fmt.Println("handled preflight")
//         return
//     }
//
//     w.Header().Set("Access-Control-Allow-Origin", "*")
// }

var state = State{false, false, false}

func main() {
    http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
        http.ServeFile(w, r, "index.html")
    })

    http.HandleFunc("/index.js", func(w http.ResponseWriter, r *http.Request) {
        http.ServeFile(w, r, "index.js")
    })

	http.HandleFunc("/web/pc", func(w http.ResponseWriter, r *http.Request) {
        if state.pc {
            fmt.Fprintf(w, "on")
        } else {
            fmt.Fprintf(w, "off")
        }
	})

    http.HandleFunc("/web/pc/request-power-on", func(w http.ResponseWriter, r *http.Request) {
        state.to_turn_on = true
        fmt.Fprint(w, "requested")
    })

    http.HandleFunc("/web/pc/request-reset", func(w http.ResponseWriter, r *http.Request) {
        state.to_reset = true
        fmt.Fprint(w, "requested reset")
    })

    http.HandleFunc("/esp/pc/mark-as-on", func(w http.ResponseWriter, r *http.Request) {
        state.pc = true
        fmt.Fprint(w, "marked pc as on")
    })

    http.HandleFunc("/esp/pc/mark-as-off", func(w http.ResponseWriter, r *http.Request) {
        state.pc = false
        fmt.Fprint(w, "marked pc as off")
    })

    http.HandleFunc("/esp/pc/check-request", func(w http.ResponseWriter, r *http.Request) {
        if state.to_turn_on {
            fmt.Fprint(w, 1)
        } else {
            fmt.Fprint(w, 0)
        }
    })

    http.HandleFunc("/esp/pc/request-fulfilled", func(w http.ResponseWriter, r *http.Request) {
        state.to_turn_on = false
        fmt.Fprint(w, "marked power on request as fulfilled")
    })

    http.HandleFunc("/esp/pc/check-reset-request", func(w http.ResponseWriter, r *http.Request) {
        if state.to_reset {
            fmt.Fprint(w, 1)
        } else {
            fmt.Fprint(w, 0)
        }
    })

    http.HandleFunc("/esp/pc/reset-request-fulfilled", func(w http.ResponseWriter, r *http.Request) {
        state.to_reset = false
        fmt.Fprint(w, "marked reset request as fulfilled")
    })

	fmt.Printf("Starting server on port %d\n", port)
	err := http.ListenAndServe(fmt.Sprintf(":%d", port), nil)

	if errors.Is(err, http.ErrServerClosed) {
		fmt.Printf("Server closed\n")
	} else if err != nil {
		fmt.Printf("[Error] Could not start server: %s\n", err)
	}
}
