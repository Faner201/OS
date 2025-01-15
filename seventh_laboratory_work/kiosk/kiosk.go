package main

import (
	"log"
	"os/exec"
	"time"

	"github.com/BurntSushi/xgb"
	"github.com/BurntSushi/xgb/xproto"
)

func main() {
	go interceptAltF4()

	for {
		err := startKioskApp()
		if err != nil {
			log.Fatalf("Error starting application: %v", err)
		}
	}
}

func startKioskApp() error {
	appPath := "../temperature_gui"
	cmd := exec.Command(appPath)

	err := cmd.Start()
	if err != nil {
		return err
	}

	time.Sleep(2 * time.Second)
	exec.Command("wmctrl", "-r", ":ACTIVE:", "-b", "add,fullscreen").Run()
	exec.Command("wmctrl", "-r", ":ACTIVE:", "-b", "add,undecorated").Run()

	return cmd.Wait()
}

func interceptAltF4() {
	conn, err := xgb.NewConn()
	if err != nil {
		log.Fatalf("Failed to connect to X server: %v", err)
	}
	defer conn.Close()

	setup := xproto.Setup(conn)
	root := setup.DefaultScreen(conn).Root

	xproto.ChangeWindowAttributes(conn, root, xproto.CwEventMask, []uint32{xproto.EventMaskKeyPress})

	keycodeF4 := getKeycode(conn, "F4")

	for {
		event, err := conn.WaitForEvent()
		if err != nil {
			log.Fatalf("Error waiting for event: %v", err)
		}

		switch e := event.(type) {
		case xproto.KeyPressEvent:
			if e.Detail == keycodeF4 && (e.State&xproto.ModMask1) != 0 {
				return
			}
		}
	}
}

func stringToKeysym(key string) xproto.Keysym {
	switch key {
	case "F4":
		return xproto.Keysym(0xFFBE)
	default:
		log.Fatalf("Keysym for %s not found", key)
		return 0
	}
}

func getKeycode(conn *xgb.Conn, key string) xproto.Keycode {
	reply, err := xproto.GetKeyboardMapping(conn, xproto.Keycode(8), 255).Reply()
	if err != nil {
		log.Fatalf("Error getting keyboard mapping: %v", err)
	}
	keysym := stringToKeysym(key)

	for i, ks := range reply.Keysyms {
		if ks == keysym {
			return xproto.Keycode(i + 8)
		}
	}

	log.Fatalf("Keycode for %s not found", key)
	return 0
}
