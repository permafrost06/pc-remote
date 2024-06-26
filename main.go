package main

import (
	"errors"
	"fmt"
	"net/http"
	"os"
	"slices"
	"strings"

	"github.com/joho/godotenv"
)

const port = 8000

type State struct {
	pc                  bool
	to_turn_on          bool
	to_press_pwr_button bool
	to_reset            bool
}

var state = State{false, false, false, false}

func main() {
	err := godotenv.Load()

	var approved_secrets = strings.Split(os.Getenv("PC_REMOTE_SECRETS"), ",")

	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, "index.html")
	})

	http.HandleFunc("/index.js", func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, "index.js")
	})

	http.HandleFunc("/set-code", func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, "code.html")
	})

	http.HandleFunc("/web/pc", func(w http.ResponseWriter, r *http.Request) {
		if !slices.Contains(approved_secrets, r.Header.Get("Secret-Code")) {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}

		if state.pc {
			fmt.Fprintf(w, "on")
		} else {
			fmt.Fprintf(w, "off")
		}
	})

	http.HandleFunc("/esp/pc/mark-as-on", func(w http.ResponseWriter, r *http.Request) {
		if !slices.Contains(approved_secrets, r.Header.Get("Secret-Code")) {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}

		state.pc = true
		fmt.Fprint(w, "marked pc as on")
	})

	http.HandleFunc("/esp/pc/mark-as-off", func(w http.ResponseWriter, r *http.Request) {
		if !slices.Contains(approved_secrets, r.Header.Get("Secret-Code")) {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}

		state.pc = false
		fmt.Fprint(w, "marked pc as off")
	})

	http.HandleFunc("/web/pc/request-power-on", func(w http.ResponseWriter, r *http.Request) {
		if !slices.Contains(approved_secrets, r.Header.Get("Secret-Code")) {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}

		state.to_turn_on = true
		fmt.Fprint(w, "requested")
	})

	http.HandleFunc("/esp/pc/check-request", func(w http.ResponseWriter, r *http.Request) {
		if !slices.Contains(approved_secrets, r.Header.Get("Secret-Code")) {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}

		if state.to_turn_on {
			fmt.Fprint(w, 1)
		} else {
			fmt.Fprint(w, 0)
		}
	})

	http.HandleFunc("/esp/pc/request-fulfilled", func(w http.ResponseWriter, r *http.Request) {
		if !slices.Contains(approved_secrets, r.Header.Get("Secret-Code")) {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}

		state.to_turn_on = false
		fmt.Fprint(w, "marked power on request as fulfilled")
	})

	http.HandleFunc("/web/pc/request-power-button-press", func(w http.ResponseWriter, r *http.Request) {
		if !slices.Contains(approved_secrets, r.Header.Get("Secret-Code")) {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}

		state.to_press_pwr_button = true
		fmt.Fprint(w, "requested")
	})

	http.HandleFunc("/esp/pc/check-power-button-request", func(w http.ResponseWriter, r *http.Request) {
		if !slices.Contains(approved_secrets, r.Header.Get("Secret-Code")) {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}

		if state.to_press_pwr_button {
			fmt.Fprint(w, 1)
		} else {
			fmt.Fprint(w, 0)
		}
	})

	http.HandleFunc("/esp/pc/power-button-request-fulfilled", func(w http.ResponseWriter, r *http.Request) {
		if !slices.Contains(approved_secrets, r.Header.Get("Secret-Code")) {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}

		state.to_press_pwr_button = false
		fmt.Fprint(w, "marked button press request as fulfilled")
	})

	http.HandleFunc("/web/pc/request-reset", func(w http.ResponseWriter, r *http.Request) {
		if !slices.Contains(approved_secrets, r.Header.Get("Secret-Code")) {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}

		state.to_reset = true
		fmt.Fprint(w, "requested reset")
	})

	http.HandleFunc("/esp/pc/check-reset-request", func(w http.ResponseWriter, r *http.Request) {
		if !slices.Contains(approved_secrets, r.Header.Get("Secret-Code")) {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}

		if state.to_reset {
			fmt.Fprint(w, 1)
		} else {
			fmt.Fprint(w, 0)
		}
	})

	http.HandleFunc("/esp/pc/reset-request-fulfilled", func(w http.ResponseWriter, r *http.Request) {
		if !slices.Contains(approved_secrets, r.Header.Get("Secret-Code")) {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}

		state.to_reset = false
		fmt.Fprint(w, "marked reset request as fulfilled")
	})

	fmt.Printf("Starting server on port %d\n", port)
	err = http.ListenAndServe(fmt.Sprintf(":%d", port), nil)

	if errors.Is(err, http.ErrServerClosed) {
		fmt.Printf("Server closed\n")
	} else if err != nil {
		fmt.Printf("[Error] Could not start server: %s\n", err)
	}
}
