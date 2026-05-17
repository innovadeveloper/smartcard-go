@startuml
autonumber
!theme mars
participant Host
participant SAM
participant PICC_A
participant PICC_B

note right of Host: El MANAGE CHANNEL puede estar limitado por el lector\ny emplear solamente un canal por defecto siempre '0'\n.La apertura de canales lógicos solo servirá si el firmware\ndel lector lo permite y la SAM AV2/AV3 también.
== Inicial (Canal 0 por defecto) ==

Host -> SAM: MANAGE CHANNEL (open ch1)
note right: CLA=0x80, INS=0x70
SAM --> Host: Channel = 1

Host -> SAM: MANAGE CHANNEL (open ch2)
note right: CLA=0x80, INS=0x70
SAM --> Host: Channel = 2

Host -> SAM: MANAGE CHANNEL (open ch3)
note right: CLA=0x80, INS=0x70
SAM --> Host: Channel = 3

== Canal 1 (PICC_A autenticación) ==

Host -> SAM: Auth PICC init
note right: CLA=0x81, INS=0x0A

SAM -> PICC_A: Authenticate Step 1
note right: RF exchange (no CLA ISO7816)

PICC_A --> SAM: Challenge response

SAM --> Host: Auth PICC step result
note right: CLA=0x81, INS=0x0A

== Canal 2 (PICC_B autenticación) ==

Host -> SAM: Auth PICC init
note right: CLA=0x82, INS=0x0A

SAM -> PICC_B: Authenticate Step 1
note right: RF exchange

PICC_B --> SAM: Challenge response

SAM --> Host: Auth PICC step result
note right: CLA=0x82, INS=0x0A

== Canal 3 (Host authentication / crypto) ==

Host -> SAM: Auth Host
note right: CLA=0x83, INS=0x0A

SAM --> Host: Auth response
note right: CLA=0x83, INS=0x0A

== Uso posterior de sesiones ==

Host -> SAM: Generate MAC (PICC_A context)
note right: CLA=0x81, INS=0x7C

Host -> SAM: Generate MAC (PICC_B context)
note right: CLA=0x82, INS=0x7C

Host -> SAM: Encrypt data (Host session)
note right: CLA=0x83, INS=0x7C
@enduml