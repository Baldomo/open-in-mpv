package open_in_mpv

import "net"

const defaultSocket = "/tmp/mpvsocket"

var defaultIpc = Ipc{
	SocketAddress: defaultSocket,
}

// Defines an IPC connection with a UNIX socket
type Ipc struct {
	conn          net.Conn
	SocketAddress string
}

// Send a byte-encoded command to the specified UNIX socket
func (i *Ipc) Send(cmd []byte) error {
	var err error
	i.conn, err = net.Dial("unix", i.SocketAddress)
	if err != nil {
		return err
	}
	defer i.conn.Close()

	// The command has to be newline terminated
	if cmd[len(cmd)-1] != '\n' {
		cmd = append(cmd, '\n')
	}

	_, err = i.conn.Write(cmd)
	if err != nil {
		return err
	}

	return nil
}

// Generic public send string command for the default connection
func SendString(cmd string) error {
	return defaultIpc.Send([]byte(cmd))
}

// Generic public send byte-encoded string command for the default connection
func SendBytes(cmd []byte) error {
	return defaultIpc.Send(cmd)
}
