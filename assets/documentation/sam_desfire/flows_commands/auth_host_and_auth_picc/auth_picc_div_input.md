@startuml
autonumber
!theme mars
title DESFire Authenticate PICC usando SAM

participant Reader
participant PICC
participant SAM

== Inicio autenticación ==

Reader -> PICC: Authenticate AES (KeyNo)
note right
CLA: 90
INS: AA (Authenticate AES)
end note

PICC --> Reader: RndB_enc + 91AF
note left
Respuesta de challenge (RndB cifrado)
SW: 91 AF
end note

== Envío a SAM ==

Reader -> SAM: AuthenticatePICC (Part 1)
note right
CLA: 80
INS: 0A
Datos:
- KeyNo
- RndB_enc (PICC)
- divInput (Deriva K luego)
end note

SAM --> Reader: SAM response + 90AF
note left
Contiene:
- RndA generado
- RndB procesado
SW: 90 AF
end note

== Continuación con PICC ==

Reader -> PICC: Continuation (RndA + RndB')
note right
CLA: 90
INS: AF
end note

PICC --> Reader: RndA' + 9100 / 91AE
note left
Si OK:
SW: 91 00
Si falla:
SW: 91 AE
end note

== Validación final con SAM ==

Reader -> SAM: AuthenticatePICC (Part 2)
note right
CLA: 80
INS: 0A
Datos:
- Respuesta PICC (RndA')
end note

SAM --> Reader: 9000
note left
Autenticación completada en SAM
end note

== Obtener Session Key ==

Reader -> SAM: Get Session Key
note right
CLA: 80
INS: D5
end note

SAM --> Reader: Session Key + 9000
note left
Clave de sesión AES derivada
end note
@enduml