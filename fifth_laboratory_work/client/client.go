package main

import (
	"fmt"
	"io"
	"log"
	"net/http"
)

func GetData(url string) {
	resp, err := http.Get(url)
	if err != nil {
		log.Fatalf("Error sending request: %v", err)
	}
	defer resp.Body.Close()

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		log.Fatalf("Error reading response: %v", err)
	}

	fmt.Println("Response from server:")
	fmt.Println(string(body))
}

func main() {
	GetData("http://localhost:8080/temperature")
	GetData(fmt.Sprintf("http://localhost:8080/statistics/hourly?day=%s&hour=%s", "01", "12"))
	GetData(fmt.Sprintf("http://localhost:8080/statistics/daily?month=%s&day=%s", "01", "01"))
}
