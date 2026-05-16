@startuml
!theme mars
title Personalización de tarjeta DESFire (diversificación con SAM)

participant IssuerSystem as "Issuer / Personalization System"
participant Reader
participant SAM
participant PICC

== Inicialización ==

IssuerSystem -> Reader: Insertar tarjeta
Reader -> PICC: Get UID
note right
CLA: 90
INS: 51 (GetCardUID - depende config)
end note

PICC --> Reader: UID + 9100

Reader -> IssuerSystem: UID leído

== Cálculo de divInput ==

IssuerSystem -> IssuerSystem: Construir divInput\n(ej: UID || UID || constantes)

== Derivación de clave en SAM ==

Reader -> SAM: Diversify Key
note right
CLA: 80
INS: 0A
Modo: derivación (sin PICC)
Datos:
- MasterKey (referencia)
- divInput
end note

SAM --> Reader: DiversifiedKey (o estado interno) + 9000

== Autenticación inicial con PICC ==

Reader -> PICC: Authenticate (Key Master PICC)
note right
CLA: 90
INS: AA (AES Authenticate)
end note

PICC --> Reader: RndB + 91AF

Reader -> SAM: AuthenticatePICC (Part 1)
note right
CLA: 80
INS: 0A
Datos:
- RndB
- divInput
end note

SAM --> Reader: RndA + respuesta + 90AF

Reader -> PICC: Continuation
note right
CLA: 90
INS: AF
end note

PICC --> Reader: RndA' + 9100

Reader -> SAM: AuthenticatePICC (Part 2)
note right
CLA: 80
INS: 0A
end note

SAM --> Reader: 9000

== Cambio de clave en PICC ==

Reader -> PICC: ChangeKey (nueva clave diversificada)
note right
CLA: 90
INS: C4 (Change Key)
Datos:
- KeyNo
- Nueva clave (Diversify Key)
end note

PICC --> Reader: 9100

== Verificación (opcional) ==

Reader -> PICC: Authenticate con nueva clave
note right
CLA: 90
INS: AA
end note

PICC --> Reader: OK (9100)

== Finalización ==

IssuerSystem -> Reader: Tarjeta personalizada lista
@enduml