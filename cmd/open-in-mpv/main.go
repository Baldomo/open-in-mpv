package main

import (
	"fmt"
	"log"
	"os"
	"os/exec"

	oim "github.com/Baldomo/open-in-mpv"
)

func must(err error) {
	if err == nil {
		return
	}

	log.Fatal(err.Error())
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("This program is not supposed to be called from the command line!")
		os.Exit(1)
	}

	must(oim.LoadConfig())

	opts := oim.NewOptions()
	must(opts.Parse(os.Args[1]))

	if opts.NeedsIpc {
		cmd, err := opts.GenerateIPC()
		must(err)
		err = oim.SendBytes(cmd)
		if err == nil {
			os.Exit(0)
		}
		log.Println("Error writing to socket, opening new instance")
	}

	executable, args := opts.GenerateCommand()
	player := exec.Command(executable, args...)
	log.Println(player.String())
	must(player.Start())
	// must(player.Wait())
}
