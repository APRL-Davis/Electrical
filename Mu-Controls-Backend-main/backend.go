package main

import (
	"fmt"
	"net"
	"net/http"
	"os"
	"strings"
	"github.com/gin-gonic/gin"
)

func udp_manage (connection *net.UDPConn, buffer []byte) {

	for {
		n, addr, err := connection.ReadFromUDP(buffer)
		data := string(buffer[0:n-1])
		fmt.Print("-> ", data, "\n")

		if strings.TrimSpace(string(buffer[0:n])) == "STOP" {
			fmt.Println("Exiting UDP server!")
			return
		}

		fmt.Printf("Received data: %s\n", data)
		_, err = connection.WriteToUDP([]byte("Received data: "+data), addr)
		if err != nil {
			fmt.Println(err)
			return
		}
	}
}

func main() {
	arguments := os.Args
	if len(arguments) == 1 {
		fmt.Println("Please provide port number")
		return
	}

	PORT := ":" + arguments[1]
	
	r := gin.Default()

	s, err := net.ResolveUDPAddr("udp4", PORT)
	if err != nil {
		fmt.Println(err)
		return
	}

	connection, err := net.ListenUDP("udp4", s)
	if err != nil {
		fmt.Println(err)
		return
	}

	defer connection.Close()
	buffer := make([]byte, 1024)

	go udp_manage(connection, buffer)

	r.GET("/ping", func(c *gin.Context) {
		c.JSON(http.StatusOK, gin.H{
			"message": "pong",
		})
	})

	r.POST("live-runs/new", func (c *gin.Context) {
		c.JSON(http.StatusOK, gin.H{
			"message": "This endpoint will create a new live run",
		})
	})

	r.PATCH("live-runs/:run_id", func (c *gin.Context) {
		run_id := c.Param("run_id")
		c.JSON(http.StatusOK, gin.H{
			"message": "You just updated run id: "+run_id,
		})
	})

	r.Run()
}
