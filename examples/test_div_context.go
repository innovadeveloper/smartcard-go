package main

import (
	"encoding/hex"
	"fmt"
)

// buildDivInput constructs the 16-byte diversification input for SAM AuthenticatePICC.
//
// Formula: UID || 0x00 || NOT(UID) || 0xFF
//
// The SAM uses this as the div-input for AES key diversification (AN10922-style).
// Byte 8 is always 0xFB because all NXP DESFire tags start with UID[0]=0x04.
func buildDivInput(uid []byte) []byte {
	divInput := make([]byte, 16)
	copy(divInput[0:7], uid) // bytes 0-6: UID
	divInput[7] = 0x00       // byte 7: separator
	for i := 0; i < 7; i++ {
		divInput[8+i] = ^uid[i] // bytes 8-14: bitwise NOT of UID
	}
	divInput[15] = 0xFF // byte 15: terminator
	return divInput
}

func verify(name string, uidHex string, expectedSuffixHex string) {
	uid, _ := hex.DecodeString(uidHex)
	divInput := buildDivInput(uid)

	// expectedSuffixHex is bytes 7-15 of divInput (the part after the UID)
	fullExpected := uidHex + expectedSuffixHex
	computed := fmt.Sprintf("%x", divInput)

	match := computed == fullExpected
	status := "OK"
	if !match {
		status = "FAIL"
	}

	fmt.Printf("[%s] %s\n", status, name)
	fmt.Printf("  UID:      %s\n", uidHex)
	fmt.Printf("  divInput: %s\n", computed)
	fmt.Printf("  expected: %s\n", fullExpected)
	fmt.Println()
}

func main() {
	verify("case1", "042f76ca8e2390", "00fbd0893571dc6fff")
	verify("case2", "04422112606f80", "00fbbddeed9f907fff")
	verify("case3", "045d72ca8e2390", "00fba28d3571dc6fff")
	verify("case4", "04ab57ca8e2390", "00fb54a83571dc6fff")
}
