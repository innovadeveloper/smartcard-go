package main

import (
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
		
		// Authenticate SAM host
		err = authenticateSam(sam)
		if err != nil {
			log.Printf("Failed to authenticate SAM: %v", err)
			continue
		}
		
		log.Println("SAM connection, version reading, and authentication successful!")
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