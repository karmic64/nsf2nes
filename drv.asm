tuneid  = $01ff
rawjoy  = $01fe
region  = $01fd
nmiflag = $01fc
stack   = $01fb


init    = $4242
play    = $4343
maxtune = $44
deftune = $45
        
        
        .logical $4100
        inc nmiflag
        
        plp
        sei
        php
        rti
        
        
        ldx #<stack
        txs
-       bit $2002
        bpl *-3
        inx
        bne -
        lda #$80
        sta $2000
        ldy #0
        sty rawjoy
        lda nmiflag
-       cmp nmiflag
        bne +
        inx
        bne -
        iny
        bne -
+       cpy #$b
        lda #0
        rol
        sta region
        lda #deftune
        sta tuneid
        
clear   lda #7
        sta 1
        lda #0
        sta $4015
        sta $4017
        tay
        sta 0
        ldx #6
-       sta (0),y
        iny
        bne -
        dec 1
        dex
        bne -
-       sta 0,x
        sta @wstack-$ff,x
        inx
        bne -
        lda #$0f
        sta $4015
        lda tuneid
        ldx region
        jsr init
        
main    lda nmiflag
        cmp nmiflag
        beq *-3
        jsr play
        
        ldy rawjoy
        jsr readjoy
-       lda rawjoy
        pha
        jsr readjoy
        pla
        cmp rawjoy
        bne -
        tya
        eor #$ff
        and rawjoy
        tay
        ldx tuneid
        and #$29
        beq +
        inx
        cpx #maxtune
        bcc +
        ldx #0
+       tya
        and #$06
        beq ++
        txa
        bne +
        ldx #maxtune
+       dex
+       stx tuneid
        tya
        bne clear
        beq main
        
        
readjoy ldx #1
        stx $4016
        stx rawjoy
        dex
        stx $4016
-       lda $4016
        and #3
        cmp #1
        rol rawjoy
        bcc -
        rts
        
        
        
        .here
        
        