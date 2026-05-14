> idnciame los pasos de como compilar y probar ese script de go creado.. y como implementa la librería

⏺ Pasos para compilar y probar el script

  1. Compilación

  # Desde el directorio examples/
  cd /Users/kenny/Projects/GoProjects/smartcard-go/examples
  go build test_sam_connection.go

  2. Ejecución

  # Ejecutar el binario generado
  ./test_sam_connection

  O compilar y ejecutar en un solo paso:
  go run test_sam_connection.go

  Implementación de la librería

  Estructura del código

  1. Importaciones necesarias:
  import (
      "smart_project/pcsc"              // Para contexto PCSC y lectores
      "smart_project/nxp/mifare/samav2" // Para funcionalidad SAM AV2
  )

  2. Inicialización del contexto PCSC:
  ctx, err := pcsc.NewContext()  // Crea contexto para PCSC
  defer ctx.Release()            // Libera recursos al finalizar

  3. Descubrimiento de lectores:
  readers, err := pcsc.ListReaders(ctx)  // Lista todos los lectores disponibles

  4. Creación de objetos Reader:
  reader := pcsc.NewReader(ctx, readerName)  // Crea reader para cada dispositivo

  5. Conexión al SAM:
  sam, err := samav2.ConnectSamAv2(reader)  // Conecta específicamente a SAM AV2
  defer sam.DisconnectCard()                // Desconecta al finalizar

  6. Operaciones con el SAM:
  version, err := sam.GetVersion()  // Obtiene versión del SAM

  Jerarquía de paquetes utilizados

  smart_project/
  ├── pcsc/              → Comunicación PCSC básica
  │   ├── NewContext()   → Inicializa contexto
  │   ├── ListReaders()  → Lista lectores
  │   └── NewReader()    → Crea objeto reader
  │
  └── nxp/mifare/samav2/ → Funcionalidad específica SAM AV2
      ├── ConnectSamAv2() → Conecta al SAM
      ├── GetVersion()    → Lee versión
      ├── AuthHostAV2()   → Autenticación
      └── [otros métodos] → Operaciones criptográficas

  Interfaces clave

  IReader interface (smartcard.IReader):
  - ConnectCard() - Conecta tarjeta normal
  - ConnectSamCard() - Conecta SAM con protocolo T=1
  - ConnectSamCard_T0() - Conecta SAM con protocolo T=0

  SamAv2 interface:
  - GetVersion() - Obtiene información del SAM
  - AuthHostAV2() - Autenticación host AV2
  - DumpSessionKey() - Extrae claves de sesión
  - SAMEncipherData() - Cifrado de datos

  Flujo de ejecución exitoso

  1. Inicialización → Contexto PCSC creado
  2. Descubrimiento → Encuentra 3 slots del ACR1581U
  3. Conexión → SAM detectado en slot 3
  4. Comunicación → GetVersion() exitoso
  5. Resultado → Información del SAM AV2 mostrada

  El código implementa el patrón estándar de la librería: contexto → reader → conexión específica → operaciones.
