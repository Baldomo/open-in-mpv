package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"os/exec"

	oim "github.com/Baldomo/open-in-mpv"
)

var (
	Version   = "dev"
	Commit    string
	BuildDate string
)

func must(err error) {
	if err == nil {
		return
	}

	log.Fatal(err.Error())
}

func main() {
	flag.Usage = func() {
		fmt.Printf("Usage: open-in-mpv [OPTIONS] <URL>\n")
		fmt.Printf("Flags:\n")
		flag.PrintDefaults()
	}

	showVersion := flag.Bool("v", false, "show version information")
	flag.Parse()

	if *showVersion {
		fmt.Printf("open-in-mpv %s\n", Version)
		fmt.Printf("  Commit: %s\n", Commit)
		fmt.Printf("  Built on: %s\n", BuildDate)
		os.Exit(0)
	}

	if flag.NArg() == 0 {
		fmt.Println("No arguments supplied!")
		flag.Usage()
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
