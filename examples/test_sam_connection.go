package main

import (
	"fmt"
	"log"
	"strings"
	"smart_project/pcsc"  // PCSC package for context and readers
	"smart_project/nxp/mifare/samav2"  // SAMav2 package
)

func main() {
	log.Println("Starting SAM connection test with ACR1581U...")
	
	// Create pcsc context
	ctx, err := pcsc.NewContext()
	if err != nil {
		log.Fatal("Failed to create pcsc context:", err)
	}
	defer ctx.Release()
	
	// List all available readers
	readers, err := pcsc.ListReaders(ctx)
	if err != nil {
		log.Fatal("Failed to list readers:", err)
	}
	
	log.Printf("Found %d reader(s):", len(readers))
	for i, reader := range readers {
		log.Printf("  [%d]: %s", i, reader)
	}
	
	// log.Printf("sample 3")
	// Find SAM readers (looking for ACR1581U SAM slots)
	samReaders := make([]pcsc.Reader, 0)
	for _, readerName := range readers {
		// ACR1581U typically shows as "ACS ACR1581U 00 01" for slot 1 (SAM)
		if strings.Contains(strings.ToUpper(readerName), "SAM") || 
		//    strings.Contains(readerName, "01") || 
		   strings.Contains(readerName, "ACS ACR1581 1S Dual Reader(3)") { // Slot 1 is typically SAM
			log.Printf("Found potential SAM reader: %s", readerName)
			samReaders = append(samReaders, pcsc.NewReader(ctx, readerName))
		}
	}
	
	if len(samReaders) == 0 {
		log.Println("No SAM readers found. Trying all readers...")
		// If no obvious SAM reader found, try all readers
		for _, readerName := range readers {
			samReaders = append(samReaders, pcsc.NewReader(ctx, readerName))
		}
	}
	
	// Try to connect to SAM on each reader
	for i, samReader := range samReaders {
		log.Printf("Attempting SAM connection on reader %d...", i)
		
		sam, err := samav2.ConnectSamAv2(samReader)
		if err != nil {
			log.Printf("Failed to connect to SAM on reader %d: %v", i, err)
			continue
		}
		defer sam.DisconnectCard()
		
		log.Printf("Successfully connected to SAM on reader %d!", i)
		
		// Authenticate SAM host
		err = authenticateSam(sam)
		if err != nil {
			log.Printf("Failed to authenticate SAM: %v", err)
			continue
		}

		// Get SAM version
		version, err := sam.GetVersion()
		if err != nil {
			log.Printf("Failed to get SAM version: %v", err)
			continue
		}
		
		log.Printf("SAM Version: % X", version)
		
		// Parse version information if possible
		if len(version) >= 7 {
			log.Printf("  Vendor ID: %02X", version[0])
			log.Printf("  Type: %02X", version[1]) 
			log.Printf("  Subtype: %02X", version[2])
			log.Printf("  Major Version: %02X", version[3])
			log.Printf("  Minor Version: %02X", version[4])
			log.Printf("  Storage Size: %02X", version[5])
			log.Printf("  Protocol: %02X", version[6])
		}
		
		
		// Check available keys in SAM
		// log.Println("Checking available keys in SAM...")
		// for keyNo := 0; keyNo < 20; keyNo++ {
		// 	keyInfo, err := sam.SAMGetKeyEntry(keyNo)
		// 	if err == nil {
		// 		log.Printf("Key %d exists: % X", keyNo, keyInfo)
		// 	}
		// }

		// log.Println("Preparing SAM key for PICC operations...")
		// _, err2 := sam.SAMGetKeyEntry(9)
		// if err2 != nil {
		// 	log.Printf("Warning: Could not prepare key 9: %v", err2)
		// }

		// Now authenticate PICC using SAM crypto engine
		err = authenticatePiccWithSam(sam)
		if err != nil {
			log.Printf("Failed to authenticate PICC with SAM: %v", err)
			continue
		}
		
		log.Println("SAM connection, authentication, and PICC authentication successful!")
		return
	}
	
	log.Println("Failed to connect to any SAM device")
}

func authenticateSam(sam samav2.SamAv2) error {
	// SAM authentication parameters
	authKey := []byte{
		0xDB, 0x2E, 0x9E, 0x71,
		0x6C, 0x61, 0xA7, 0xCA,
		0xF9, 0x60, 0x55, 0x35,
		0xFB, 0xF0, 0x25, 0x34,
	}
	keyNo := byte(100)
	keyVer := byte(0)
	
	log.Printf("Authenticating SAM with key number %d, key version %d, authKey [% X...]", keyNo, keyVer, authKey[0:8])
	
	// Perform host authentication (AV2 mode with hostMode 2 = Full)
	_, err := sam.AuthHostAV2(authKey, int(keyNo), int(keyVer), 2)
	if err != nil {
		return err
	}
	
	log.Println("SAM host authentication successful!")
	return nil
}

func authenticatePiccWithSam(sam samav2.SamAv2) error {
	// Connect to PICC reader (typically slot 0 for ACR1581U)
	ctx, err := pcsc.NewContext()
	if err != nil {
		return fmt.Errorf("failed to create PICC context: %v", err)
	}
	defer ctx.Release()
	
	readers, err := pcsc.ListReaders(ctx)
	if err != nil {
		return fmt.Errorf("failed to list PICC readers: %v", err)
	}
	
	// Find PICC reader (typically slot 0)
	var piccReader pcsc.Reader
	for _, readerName := range readers {
		// if strings.Contains(readerName, "ACS ACR1581 1S Dual Reader") && !strings.Contains(readerName, "(3)") {
		if strings.Contains(readerName, "ACS ACR1581 1S Dual Reader(1)") {
			log.Printf("Found PICC reader: %s", readerName)
			piccReader = pcsc.NewReader(ctx, readerName)
			break
		}
	}
	
	if piccReader == nil {
		return fmt.Errorf("no PICC reader found")
	}
	
	// Connect to PICC using PCSC interface (contactless)
	piccCard, err := piccReader.ConnectCardPCSC()
	if err != nil {
		return fmt.Errorf("failed to connect to PICC: %v", err)
	}
	defer piccCard.DisconnectCard()
	
	log.Println("Connected to PICC successfully")
	
	// PICC SELECT AID (F21060)
	selectAid := []byte{0x90, 0x5A, 0x00, 0x00, 0x03, 0xF2, 0x10, 0x60, 0x00}
	response, err := piccCard.Apdu(selectAid)
	if err != nil {
		return fmt.Errorf("failed to select PICC AID: %v", err)
	}
	log.Printf("PICC SELECT AID response: % X", response)
	
	// PICC GET VERSION (90 60)
	getVersion := []byte{0x90, 0x60, 0x00, 0x00, 0x00}
	response, err = piccCard.Apdu(getVersion)
	if err != nil {
		return fmt.Errorf("failed to get PICC version: %v", err)
	}
	log.Printf("PICC GET VERSION response: % X", response)
	
	// Continue with additional frames if needed (90 AF)
	for response[len(response)-1] == 0xAF {
		continueCmd := []byte{0x90, 0xAF, 0x00, 0x00, 0x00}
		response, err = piccCard.Apdu(continueCmd)
		if err != nil {
			return fmt.Errorf("failed to continue PICC version: %v", err)
		}
		log.Printf("PICC VERSION continue: % X", response)
	}
	
	// PICC AUTH AES (90 AA) - key 0x07
	authCmd := []byte{0x90, 0xAA, 0x00, 0x00, 0x01, 0x07, 0x00}
	response, err = piccCard.Apdu(authCmd)
	if err != nil {
		return fmt.Errorf("failed to start PICC authentication: %v", err)
	}
	log.Printf("PICC AUTH AES response: % X", response)
	
	if len(response) < 16 || response[len(response)-1] != 0xAF {
		return fmt.Errorf("invalid PICC auth response: % X", response)
	}
	
	// Extract encrypted RndB from PICC response  
	encRndB := response[0:16]
	
	// Extract UID from previous PICC VERSION response - get last 7 bytes before status
	// From logs: UID is part of the version response  
	// uid := []byte{0x04, 0x2F, 0x76, 0xCA, 0x8E, 0x23, 0x90} // From logs: 042f76ca8e2390
	
	// SAM AUTH PICC Part 1 - Based on logs structure  
	// divInput from logs: 042f76ca8e239000fbd0893571dc6fff (16 bytes exactly)
	divInput := []byte{0x04, 0x2F, 0x76, 0xCA, 0x8E, 0x23, 0x90, 0x00, 0xFB, 0xD0, 0x89, 0x35, 0x71, 0xDC, 0x6F, 0xFF}

	authMode := 0x11
	
	// Try different key numbers - first try 0x00 (default), then others
	// keyNumbers := []int{0x00, 0x07, 0x09, 0x01, 0x02, 0x03}
	keyNumbers := []int{0x09}
	// keyNumbers := []int{0x07}
	var samResponse []byte
	var successKeyNo int = -1
	
	for _, keyNo := range keyNumbers {
		log.Printf("Trying SAM AuthenticatePICC with keyNo=%d...", keyNo)
		response, err := sam.AuthenticatePICC_Part1(authMode, keyNo, 0x00, encRndB, divInput)
		if err != nil {
			log.Printf("KeyNo %d failed with error: %v", keyNo, err)
			continue
		}
		
		// Check response status
		if len(response) >= 2 {
			responseStatus := response[len(response)-2:]
			log.Printf("KeyNo %d response status: % X", keyNo, responseStatus)
			
			if responseStatus[0] == 0x90 && responseStatus[1] == 0xAF {
				log.Printf("SUCCESS! KeyNo %d works", keyNo)
				samResponse = response
				successKeyNo = keyNo
				break
			} else {
				log.Printf("KeyNo %d failed with status: % X", keyNo, responseStatus)
			}
		}
	}
	
	if successKeyNo == -1 {
		return fmt.Errorf("no valid key number found for PICC authentication")
	}
	
	log.Printf("Using keyNo=%d for PICC authentication", successKeyNo)
	log.Printf("SAM AUTH PICC Part 1 response: % X", samResponse)
	
	// Send SAM response to PICC
	piccAuthData := samResponse[0:len(samResponse)-2] // Remove status bytes
	piccContinueCmd := []byte{0x90, 0xAF, 0x00, 0x00, byte(len(piccAuthData))}
	piccContinueCmd = append(piccContinueCmd, piccAuthData...)
	piccContinueCmd = append(piccContinueCmd, 0x00)
	
	response, err = piccCard.Apdu(piccContinueCmd)
	if err != nil {
		return fmt.Errorf("failed PICC auth continue: %v", err)
	}
	log.Printf("PICC AUTH continue response: % X", response)
	
	if len(response) < 16 {
		return fmt.Errorf("invalid PICC auth continue response: % X", response)
	}
	
	// Extract PICC final response and send to SAM for Part 2
	piccFinalData := response[0:len(response)-2] // Remove status bytes
	
	// SAM AUTH PICC Part 2 - Send PICC's encrypted RndA response
	samFinalResponse, err := sam.AuthenticatePICC_Part2(piccFinalData)
	if err != nil {
		return fmt.Errorf("failed SAM AuthenticatePICC Part 2: %v", err)
	}
	log.Printf("SAM AUTH PICC Part 2 response: % X", samFinalResponse)
	
	// Dump session key from SAM
	sessionKey, err := sam.DumpSessionKey()
	if err != nil {
		return fmt.Errorf("failed to dump session key: %v", err)
	}
	log.Printf("Session Key: % X", sessionKey)
	
	log.Println("PICC authentication with SAM completed successfully!")
	return nil
}