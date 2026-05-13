⏺ Tengo suficiente información para darte el análisis completo. Aquí está:

  ---
  Análisis del Repositorio para tu Caso de Uso
  
  Soporte para ACR1581U

  El repositorio no menciona explícitamente el ACR1581U, pero sí funciona con él porque:
  - El paquete pcsc/ implementa la interfaz PC/SC genérica — cualquier lector PC/SC la usa, incluyendo el ACR1581U
  - El ACR1581U aparece en el sistema como dos lectores PC/SC:
    - ACS ACR1581 1S Dual Reader PICC 0 → ranura contactless (DESFire EV2)
    - ACS ACR1581 1S Dual Reader SAM 0 → ranura de contacto (SAM)

  ---
  Paquetes relevantes para tu flujo

  ┌─────────────────────────┬────────────────────────────────────────────────┐
  │         Paquete         │                   Propósito                    │
  ├─────────────────────────┼────────────────────────────────────────────────┤
  │ pcsc/                   │ Conexión PC/SC al lector                       │
  ├─────────────────────────┼────────────────────────────────────────────────┤
  │ nxp/mifare/samav2/      │ Autenticación y crypto con la SAM AV2          │
  ├─────────────────────────┼────────────────────────────────────────────────┤
  │ nxp/mifare/desfire/ev2/ │ Comandos DESFire EV2 (SelectApp, Authenticate) │
  └─────────────────────────┴────────────────────────────────────────────────┘

  ---
  Secuencia APDU completa: SAM → DESFire EV2
  
  Tu flujo correcto tiene 3 fases. La clave nunca sale de la SAM si usas la opción manual (Opción 2 abajo).

  ---
  FASE 1 — Autenticar el Host ante la SAM
  
  Implementado en: samav2/samAv2.go:156 — AuthHostAV1()

  # Parte 1: SAM envía rndB encriptado
  → 80 A4 [authMode] 00 02 [KEY_NO] [KEY_VER] 00
     ↑CLA ↑INS ↑P1=0(plain)/1(MAC)/2(Full) ↑P2 ↑Lc ↑tu KEY_NO ↑tu KEY_VER ↑Le
  ← [enc(rndB)] AF            # SAM responde con rndB cifrado + 0xAF (more data)

  # Parte 2: Host envía enc(rndA || rndB')
  → 80 A4 00 00 [Lc] [enc(rndA || rotate1(rndB))] 00
  ← 90 00                     # SAM autenticado, sesión establecida

  Campos de tu SAM:
  - KEY_NO = número de entrada de clave en la SAM (byte, ej: 0x01)
  - KEY_VER = versión de la clave (byte, ej: 0x00)
  - authMode = 0x00 (plain), 0x01 (MAC), 0x02 (Full)

  ---
  FASE 2 — Seleccionar la Aplicación en DESFire
  
  Implementado en: desfire/ev2/appmgmt.go:141 — SelectApplication()

  # AID = F2 10 60 (3 bytes, enviados en el orden que los configuraste)
  → 5A F2 10 60
  ← 00 90 00      # OK

  ▎ Nota sobre el AID: DESFire usa little-endian internamente pero el APDU envía los bytes en el orden que pasas al SelectApplication. Si el AID fue creado como F21060 hex literal → pasa []byte{0xF2, 0x10, 0x60}. Si fue creado como entero 0xF21060 
  ▎ en little-endian → pasa []byte{0x60, 0x10, 0xF2}. Verifica con tu configuración.

  ---
  FASE 3 — Autenticar en DESFire usando la clave de la SAM
  
  El repositorio NO tiene el comando SAM_AuthenticatePICC_MFD de NXP (el flujo "tunnel" oficial donde la SAM hace todo). Tienes dos opciones practicables:

  ---
  Opción A — SAM como oráculo criptográfico (la clave no sale, pero requiere código manual)

  # Paso 1: DESFire inicia autenticación EV2 (AES)
  → 71 [keyNo] [lenCap] [pcdCap2]     # AuthenticateEV2First, keyNo = key en DESFire
  ← [enc(rndB)] AF                    # DESFire responde con rndB cifrado

  # Paso 2: Descifrar rndB usando la SAM (clave ya cargada tras FASE 1)
  → SAM: 80 DD 00 00 [Lc] [enc(rndB)] 00    # SAMDecipherData (samav2/crypto.go:26)
  ← [rndB_plain] 90 00

  # Paso 3: Generar rndA random, rotar rndB → rndBr
  # rndBr = rotate(rndB, 1) para AES-128

  # Paso 4: Encriptar (rndA || rndBr) con la SAM
  → SAM: 80 ED 00 00 [Lc] [rndA || rndBr] 00   # SAMEncipherData (samav2/crypto.go:12)
  ← [enc(rndA || rndBr)] 90 00

  # Paso 5: Enviar respuesta a DESFire
  → AF [enc(rndA || rndBr)]           # AuthenticateEV2First Parte 2
  ← [enc(TI || rndA' || pdCap2)] 00  # DESFire confirma

  Las session keys (ksesAuthEnc, ksesAuthMac) se derivan en Go con la lógica de ev2/auth.go:268.

  ---
  Opción B — Extraer clave de la SAM (menos seguro, clave viaja en memoria)

  # Tras autenticar la SAM (FASE 1):
  sam.DumpSecretKey(KEY_NO, KEY_VER, divInput)
  # → retorna la clave AES de 16 bytes
  # → úsala directamente en AuthenticateEV2First/Part2 de ev2/auth.go

  Implementado en samav2/samAv2.go — DumpSecretKey().

  ---
  Tabla completa de APDUs del repositorio

  ┌────────────────────┬──────────────────────────────────────────┬─────────────────────┐
  │      Comando       │                   APDU                   │       Archivo       │
  ├────────────────────┼──────────────────────────────────────────┼─────────────────────┤
  │ SAM GetVersion          │ 80 60 00 00 00                           │ samAv2.go:125       │
  ├─────────────────────────┼──────────────────────────────────────────┼─────────────────────┤
  │ SAM AuthHost P1         │ 80 A4 [mode] 00 02 [KNO] [KVR] 00        │ samAv2.go:163       │
  ├─────────────────────────┼──────────────────────────────────────────┼─────────────────────┤
  │ SAM AuthHost P2           │ 80 A4 00 00 [Lc] [enc_data] 00           │ samAv2.go:191       │
  ├───────────────────────────┼──────────────────────────────────────────┼─────────────────────┤
  │ SAM LockUnlock            │ 80 10 [P1] 00 [Lc] [KNO KVR ...] 00      │ samAv2.go:221       │
  ├───────────────────────────┼──────────────────────────────────────────┼─────────────────────┤
  │ SAM EncipherData          │ 80 ED [P1] [offset] [Lc] [data] 00       │ crypto.go:12        │
  ├───────────────────────────┼──────────────────────────────────────────┼─────────────────────┤
  │ SAM DecipherData          │ 80 DD [P1] 00 [Lc] [cipher] 00           │ crypto.go:26         │
  ├───────────────────────────┼──────────────────────────────────────────┼──────────────────────┤
  │ SAM GenerateCMAC          │ 80 7C [P1] 10 [Lc] [data] 00             │ crypto.go:57         │
  ├───────────────────────────┼──────────────────────────────────────────┼──────────────────────┤
  │ SAM ChangeKeyEntry        │ 80 C1 [KNbr] [ProMax] [Lc] [enc_payload] │ keymanagement.go:18  │
  ├───────────────────────────┼──────────────────────────────────────────┼──────────────────────┤
  │ DESFire SelectApp         │ 5A [AID 3 bytes]                         │ appmgmt.go:153       │
  ├───────────────────────────┼──────────────────────────────────────────┼──────────────────────┤
  │ DESFire AuthEV2First P1   │ 71 [keyNo] [lenCap] [pcdCap2]            │ auth.go:163          │
  ├───────────────────────────┼──────────────────────────────────────────┼──────────────────────┤
  │ DESFire AuthEV2First P2   │ AF [enc(rndA||rndBr)]                    │ auth.go:183          │
  ├───────────────────────────┼──────────────────────────────────────────┼──────────────────────┤
  │ DESFire AuthISO P1 (3DES) │ 1A [keyNo]                               │ auth.go:15           │
  ├───────────────────────────┼──────────────────────────────────────────┼──────────────────────┤
  │ DESFire AuthISO P2        │ AF [enc(rndA||rndBr)]                    │ auth.go:48           │
  ├───────────────────────────┼──────────────────────────────────────────┼──────────────────────┤
  │ DESFire GetVersion        │ 60                                       │ memconfigmgmt.go     │
  ├───────────────────────────┼──────────────────────────────────────────┼──────────────────────┤
  │ DESFire GetCardUID        │ 51                                       │ memconfigmgmt.go:193 │
  └───────────────────────────┴──────────────────────────────────────────┴──────────────────────┘

  ---
  Qué falta / limitaciones críticas
  
  ┌───────────────────────────────────────────────────┬─────────────────────────────────────────────────┐
  │                        Gap                        │              Impacto para tu caso               │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ AuthenticateEV2NonFirst — no implementado (panic) │ Si necesitas re-autenticar en la misma sesión   │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ No hay flujo integrado SAM+DESFire                │ Debes orquestar manualmente las 3 fases         │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ AuthenticateEV2NonFirst — no implementado (panic) │ Si necesitas re-autenticar en la misma sesión   │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ AuthenticateEV2NonFirst — no implementado (panic) │ Si necesitas re-autenticar en la misma sesión   │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  ┌───────────────────────────────────────────────────┬─────────────────────────────────────────────────┐
  │                        Gap                        │              Impacto para tu caso               │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ AuthenticateEV2NonFirst — no implementado (panic) │ Si necesitas re-autenticar en la misma sesión   │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ AuthenticateEV2NonFirst — no implementado (panic) │ Si necesitas re-autenticar en la misma sesión   │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ AuthenticateEV2NonFirst — no implementado (panic) │ Si necesitas re-autenticar en la misma sesión   │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ AuthenticateEV2NonFirst — no implementado (panic) │ Si necesitas re-autenticar en la misma sesión   │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ AuthenticateEV2NonFirst — no implementado (panic) │ Si necesitas re-autenticar en la misma sesión   │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ AuthenticateEV2NonFirst — no implementado (panic) │ Si necesitas re-autenticar en la misma sesión   │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ AuthenticateEV2NonFirst — no implementado (panic) │ Si necesitas re-autenticar en la misma sesión   │
  │                        Gap                        │              Impacto para tu caso               │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ AuthenticateEV2NonFirst — no implementado (panic) │ Si necesitas re-autenticar en la misma sesión   │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ AuthenticateEV2NonFirst — no implementado (panic) │ Si necesitas re-autenticar en la misma sesión   │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ No hay flujo integrado SAM+DESFire                │ Debes orquestar manualmente las 3 fases         │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ DESFire AuthEV2First P2   │ AF [enc(rndA||rndBr)]                    │ auth.go:183          │
  ├───────────────────────────┼──────────────────────────────────────────┼──────────────────────┤
  │ DESFire AuthISO P1 (3DES) │ 1A [keyNo]                               │ auth.go:15           │
  ├───────────────────────────┼──────────────────────────────────────────┼──────────────────────┤
  │ DESFire AuthISO P2        │ AF [enc(rndA||rndBr)]                    │ auth.go:48           │
  ├───────────────────────────┼──────────────────────────────────────────┼──────────────────────┤
  │ DESFire GetVersion        │ 60                                       │ memconfigmgmt.go     │
  ├───────────────────────────┼──────────────────────────────────────────┼──────────────────────┤
  │ DESFire GetCardUID        │ 51                                       │ memconfigmgmt.go:193 │
  └───────────────────────────┴──────────────────────────────────────────┴──────────────────────┘

  ---
  Qué falta / limitaciones críticas

  ┌───────────────────────────────────────────────────┬─────────────────────────────────────────────────┐
  │                        Gap                        │              Impacto para tu caso               │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ AuthenticateEV2NonFirst — no implementado (panic) │ Si necesitas re-autenticar en la misma sesión   │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ No hay flujo integrado SAM+DESFire                │ Debes orquestar manualmente las 3 fases         │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ Verificación de MAC en ReadData/WriteData — TODO  │ Riesgo de integridad si lees datos en FULL mode │
  ├───────────────────────────────────────────────────┼─────────────────────────────────────────────────┤
  │ SAM_AuthenticatePICC_MFD no existe en el repo     │ El "tunnel" oficial NXP no está                 │
  └───────────────────────────────────────────────────┴─────────────────────────────────────────────────┘

  ---
  Veredicto

  El repositorio tiene los bloques necesarios pero no el flujo integrado para tu caso. Para usar tu SAM con AID F21060:

  1. Conectar con pcsc al PICC (DESFire) y al SAM por separado
  2. Autenticar host→SAM con AuthHostAV1 o AuthHostAV2
  3. Seleccionar AID en DeSFire con SelectApplication
  4. Orquestar la autenticación DESFire manualmente usando la SAM como oráculo (SAMDecipherData / SAMEncipherData), o extraer la clave con DumpSecretKey si el SAM lo permite

  ¿Quieres que te arme el código Go que integra el flujo completo de las 3 fases?