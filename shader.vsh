; $MODE = "UniformRegister"

; numOutputs - 1
; $SPI_VS_OUT_CONFIG.VS_EXPORT_COUNT = 0

; $NUM_SPI_VS_OUT_ID = 1

; position
; $SPI_VS_OUT_ID[0].SEMANTIC_0 = 0

; C0
; $UNIFORM_VARS[0].name = "uModelViewProj"
; $UNIFORM_VARS[0].type = "mat4"
; $UNIFORM_VARS[0].count = 1
; $UNIFORM_VARS[0].block = -1
; $UNIFORM_VARS[0].offset = 0

; R1
; $ATTRIB_VARS[0].name = "aPosition"
; $ATTRIB_VARS[0].type = "vec3"
; $ATTRIB_VARS[0].location = 0

; Multiply uModelViewProj and aPosition
; Pass position to pixel shader for height calculations

00 CALL_FS NO_BARRIER
01 ALU: ADDR(32) CNT(18)
    0  x: MUL    ____, 1.0f, C3.x
       y: MUL    ____, 1.0f, C3.y
       z: MUL    ____, 1.0f, C3.z
       w: MUL    ____, 1.0f, C3.w
    1  x: MULADD R127.x, R1.z, C2.x, PV0.x
       y: MULADD R127.y, R1.z, C2.y, PV0.y
       z: MULADD R127.z, R1.z, C2.z, PV0.z
       w: MULADD R127.w, R1.z, C2.w, PV0.w
    2  x: MULADD R127.x, R1.y, C1.x, PV0.x
       y: MULADD R127.y, R1.y, C1.y, PV0.y
       z: MULADD R127.z, R1.y, C1.z, PV0.z
       w: MULADD R127.w, R1.y, C1.w, PV0.w
    3  x: MULADD R2.x, R1.x, C0.x, PV0.x
       y: MULADD R2.y, R1.x, C0.y, PV0.y
       z: MULADD R2.z, R1.x, C0.z, PV0.z
       w: MULADD R2.w, R1.x, C0.w, PV0.w
02 EXP_DONE: POS0, R2
03 EXP_DONE: PARAM0, R1 NO_BARRIER
END_OF_PROGRAM
