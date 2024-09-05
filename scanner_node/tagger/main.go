package main

import (
	"bufio"
	"fmt"
	"log"
	"net/http"
	"os"
	"strings"
	"sync"

	"github.com/tarm/serial"
)

// Channel to receive next tag
var tags = make(chan string, 1)

// List of IDs we will read from the file
var listIDs []string

// Index of the next ID to use
var listIndex int

// Mutex to protect access to listIndex
var mut sync.Mutex

// Cache of the last tag to prevent duplicate reading
var lastTag string

// Mutex to protect access to lastTag
var lastTagMut sync.Mutex

// Function to load list of IDs from file
func loadListIDs(filename string) {
	file, err := os.Open(filename)
	if err != nil {
		log.Fatalf("Failed to open list file: %v", err)
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		listIDs = append(listIDs, scanner.Text())
	}

	if err := scanner.Err(); err != nil {
		log.Fatalf("Error reading list file: %v", err)
	}

	if len(listIDs) == 0 {
		log.Fatalf("No list IDs found in the file")
	}
}

// Function to manage serial port reading
func readSerialPort() {
	// Configuration for the serial port
	c := &serial.Config{Name: "/dev/ttyUSB0", Baud: 9600}

	// Attempt to open the serial port
	fmt.Println("Attempting to open Serial Port: /dev/ttyUSB0...")
	s, err := serial.OpenPort(c)
	if err != nil {
		log.Fatalf("Failed to open serial port: %v", err)
	}
	defer func() {
		fmt.Println("Closing serial port...")
		s.Close()
	}()

	fmt.Println("Serial port successfully opened. Reading...")

	// Create a buffered reader for the serial port
	scanner := bufio.NewScanner(s)

	// Continuously read from the serial port
	for scanner.Scan() {
		// Read the input from the serial port
		input := scanner.Text()

		// Ignore 'READY' messages
		if strings.Contains(input, "READY") {
			continue
		}

		// Debounce logic: ensure the same tag is not processed twice consecutively
		lastTagMut.Lock()
		if input == lastTag {
			lastTagMut.Unlock()
			continue
		}
		lastTag = input
		lastTagMut.Unlock()

		// Send the tag to the processing channel
		tags <- input
	}

	// Check for scanner errors
	if err := scanner.Err(); err != nil {
		log.Fatalf("Error reading from serial port: %v", err)
	}
}

// Function to display a message indicating the next expected list ID
func displayNextListID() {
	mut.Lock()
	nextID := listIDs[listIndex]
	mut.Unlock()
	fmt.Printf("NEXT: %s\n", nextID)
}

// Function to process the tags and handle HTTP requests
func processTags() {
	for tag := range tags {
		mut.Lock()
		listID := listIDs[listIndex]
		mut.Unlock()

		// Construct the request URL
		url := fmt.Sprintf("http://localhost:8088/link?tag=%s&list=%s", tag, listID)

		// Send the GET request
		resp, err := http.Get(url)
		if err != nil || resp.StatusCode != http.StatusOK {
			errMsg := "unknown error"
			if err != nil {
				errMsg = err.Error()
			} else {
				errMsg = fmt.Sprintf("received status code %d", resp.StatusCode)
			}
			log.Printf("Failed to send GET request to %s: %s", url, errMsg)
			if resp != nil {
				resp.Body.Close()
			}
			continue
		}
		resp.Body.Close()

		// Only proceed if the request was successful
		mut.Lock()
		listIndex = (listIndex + 1) % len(listIDs)
		mut.Unlock()

		// Print the tag and matched list ID
		fmt.Printf("Tag linkked successfully.\n")
		fmt.Println()

		// Display the next expected list ID
		displayNextListID()
	}
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: program <initial_list_id>")
		os.Exit(1)
	}
	initialListID := os.Args[1]

	loadListIDs("list.txt")

	// Find the initial list ID
	indexFound := -1
	for i, id := range listIDs {
		if id == initialListID {
			indexFound = i
			break
		}
	}

	if indexFound == -1 {
		log.Fatalf("Initial list ID %s not found in file", initialListID)
	}

	// Set the starting index
	listIndex = indexFound

	// Display the initial list ID before starting the tag read loop
	displayNextListID()

	// Start the serial port reading and tag processing
	go readSerialPort()
	processTags()
}
