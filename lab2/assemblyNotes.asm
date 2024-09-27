#compute function //a0 = 33
   0x4200bb7e <+0>:     addi    sp,sp,-32
   0x4200bb80 <+2>:     sw      s0,28(sp)
   0x4200bb82 <+4>:     addi    s0,sp,32
   0x4200bb84 <+6>:     sw      a0,-20(s0)
   0x4200bb88 <+10>:    sw      a1,-24(s0)
   0x4200bb8c <+14>:    sw      a2,-28(s0)
   0x4200bb90 <+18>:    lw      a4,-20(s0) //a4 = a0 = 33
   0x4200bb94 <+22>:    lw      a5,-24(s0) //a5 = a1
   0x4200bb98 <+26>:    add     a4,a4,a5 //a4 = a5+a4
   0x4200bb9a <+28>:    lw      a5,-28(s0) //a5 = a2
   0x4200bb9e <+32>:    add     a5,a5,a4 // a5 = a5+a4
   0x4200bba0 <+34>:    mv      a0,a5 //return a5
=> 0x4200bba2 <+36>:    lw      s0,28(sp)
   0x4200bba4 <+38>:    addi    sp,sp,32 


#app_main
   0x4200bba8 <+0>:     addi    sp,sp,-48 #sub 48 from stk ptr to make room to store values
   0x4200bbaa <+2>:     sw      ra,44(sp) #store return value on stack
   0x4200bbac <+4>:     sw      s0,40(sp) #store frame pointer on stack
   0x4200bbae <+6>:     addi    s0,sp,48 #shift stk ptr up
   0x4200bbb0 <+8>:     lui     a5,0x3c026 #load unsigned int 245798 into a5
   0x4200bbb4 <+12>:    addi    a0,a5,-1008 # 0x3c025c10 , subtract 1008 from 245798 and store in a0 (return val?)
=> 0x4200bbb8 <+16>:    jal     ra,0x42010960 <puts> #jump and link somewhere else
   0x4200bbbc <+20>:    addi    a5,s0,-40
   0x4200bbc0 <+24>:    mv      a0,a5 #frame pointer stuff prepped in a0
   0x4200bbc2 <+26>:    jal     ra,0x4200ee36 <esp_chip_info> #also jal somewher eelse
   0x4200bbc6 <+30>:    lbu     a5,-30(s0)
   0x4200bbca <+34>:    mv      a2,a5
   0x4200bbcc <+36>:    lw      a5,-36(s0)
   0x4200bbd0 <+40>:    andi    a5,a5,32
   0x4200bbd4 <+44>:    beqz    a5,0x4200bbe0 <app_main+56> //if a5 is 0, jump to flag
   0x4200bbd6 <+46>:    lui     a5,0x3c026 #load 245798 into a5
   0x4200bbda <+50>:    addi    a3,a5,-992 # 0x3c025c20, store 245798-992 into a3
   0x4200bbde <+54>:    j       0x4200bbe8 <app_main+64>
   0x4200bbe0 <+56>:    lui     a5,0x3c026 #flag, load 245798 into a5
   0x4200bbe4 <+60>:    addi    a3,a5,-988 # 0x3c025c24, store 245798-988 into a3
   0x4200bbe8 <+64>:    lw      a5,-36(s0)
   0x4200bbec <+68>:    andi    a5,a5,16
   0x4200bbee <+70>:    beqz    a5,0x4200bbfa <app_main+82>
   0x4200bbf0 <+72>:    lui     a5,0x3c026
   0x4200bbf4 <+76>:    addi    a4,a5,-984 # 0x3c025c28
   0x4200bbf8 <+80>:    j       0x4200bc02 <app_main+90>
   0x4200bbfa <+82>:    lui     a5,0x3c026
   0x4200bbfe <+86>:    addi    a4,a5,-988 # 0x3c025c24
   0x4200bc02 <+90>:    lw      a5,-36(s0)
   0x4200bc06 <+94>:    andi    a5,a5,64
   0x4200bc0a <+98>:    beqz    a5,0x4200bc16 <app_main+110>
   0x4200bc0c <+100>:   lui     a5,0x3c026
   0x4200bc10 <+104>:   addi    a5,a5,-976 # 0x3c025c30
   0x4200bc14 <+108>:   j       0x4200bc1e <app_main+118>
   0x4200bc16 <+110>:   lui     a5,0x3c026
   0x4200bc1a <+114>:   addi    a5,a5,-988 # 0x3c025c24
   0x4200bc1e <+118>:   lui     a1,0x3c026
   0x4200bc22 <+122>:   addi    a1,a1,-948 # 0x3c025c4c
   0x4200bc26 <+126>:   lui     a0,0x3c026
   0x4200bc2a <+130>:   addi    a0,a0,-940 # 0x3c025c54
   0x4200bc2e <+134>:   jal     ra,0x42010842 <printf> #prob prints hello world
   0x4200bc32 <+138>:   lhu     a4,-32(s0) //a4 = -32(s0)
   0x4200bc36 <+142>:   li      a5,100 #a5 = 100
   0x4200bc3a <+146>:   divu    a5,a4,a5 # a5 = a5/a4
   0x4200bc3e <+150>:   slli    a5,a5,0x10 
   0x4200bc40 <+152>:   srli    a5,a5,0x10 //does some unnecessary shifts here
   0x4200bc42 <+154>:   sw      a5,-20(s0) //-20(s0) = a5
   0x4200bc46 <+158>:   lhu     a4,-32(s0) //a4 = -32(s0)
   0x4200bc4a <+162>:   li      a5,100 #a5 = 100
   0x4200bc4e <+166>:   remu    a5,a4,a5 #a5 = a5%a4
   0x4200bc52 <+170>:   slli    a5,a5,0x10
   0x4200bc54 <+172>:   srli    a5,a5,0x10 //more unnecessary shifts
   0x4200bc56 <+174>:   sw      a5,-24(s0) #-24(s0) = a5
   0x4200bc5a <+178>:   lw      a2,-24(s0)
   0x4200bc5e <+182>:   lw      a1,-20(s0)
   0x4200bc62 <+186>:   lui     a5,0x3c026
   0x4200bc66 <+190>:   addi    a0,a5,-888 # 0x3c025c88
   0x4200bc6a <+194>:   jal     ra,0x42010842 <printf> //prob prints info about the board
   0x4200bc6e <+198>:   addi    a5,s0,-44
   0x4200bc72 <+202>:   mv      a1,a5
   0x4200bc74 <+204>:   li      a0,0
   0x4200bc76 <+206>:   auipc   ra,0xfe379
   0x4200bc7a <+210>:   jalr    -562(ra) # 0x40384a44 <esp_flash_get_size>
   0x4200bc7e <+214>:   mv      a5,a0
   0x4200bc80 <+216>:   beqz    a5,0x4200bc90 <app_main+232>
   0x4200bc82 <+218>:   lui     a5,0x3c026
   0x4200bc86 <+222>:   addi    a0,a5,-860 # 0x3c025ca4
   0x4200bc8a <+226>:   jal     ra,0x42010842 <printf>
   0x4200bc8e <+230>:   j       0x4200bd1e <app_main+374>
   0x4200bc90 <+232>:   lw      a5,-44(s0)
   0x4200bc94 <+236>:   srli    a4,a5,0x14
   0x4200bc98 <+240>:   lw      a5,-36(s0)
   0x4200bc9c <+244>:   andi    a5,a5,1
   0x4200bc9e <+246>:   beqz    a5,0x4200bcaa <app_main+258>
   0x4200bca0 <+248>:   lui     a5,0x3c026
   0x4200bca4 <+252>:   addi    a5,a5,-836 # 0x3c025cbc
   0x4200bca8 <+256>:   j       0x4200bcb2 <app_main+266>
   0x4200bcaa <+258>:   lui     a5,0x3c026
   0x4200bcae <+262>:   addi    a5,a5,-824 # 0x3c025cc8
   0x4200bcb2 <+266>:   mv      a2,a5
   0x4200bcb4 <+268>:   mv      a1,a4
   0x4200bcb6 <+270>:   lui     a5,0x3c026
   0x4200bcba <+274>:   addi    a0,a5,-812 # 0x3c025cd4
   0x4200bcbe <+278>:   jal     ra,0x42010842 <printf> //print "minimum free heap size: "
   0x4200bcc2 <+282>:   jal     ra,0x42019bfa <esp_get_minimum_free_heap_size>
   0x4200bcc6 <+286>:   mv      a5,a0
   0x4200bcc8 <+288>:   mv      a1,a5
   0x4200bcca <+290>:   lui     a5,0x3c026 #load 245798 into a5
   0x4200bcce <+294>:   addi    a0,a5,-796 # 0x3c025ce4, a0 = a5-796
   0x4200bcd2 <+298>:   jal     ra,0x42010842 <printf> #print minimum free heap size value
   0x4200bcd6 <+302>:   lw      a5,-20(s0) //load some val from s0 into a5
   0x4200bcda <+306>:   lw      a4,-24(s0) //load some val from s0 into a4
   0x4200bcde <+310>:   mv      a2,a4 //mov val from a4 into a2
   0x4200bce0 <+312>:   mv      a1,a5 //mov val from a5 to a1
   0x4200bce2 <+314>:   li      a0,33 //load 33 into a0?
   0x4200bce6 <+318>:   jal     ra,0x4200bb7e <compute> //does some calc
   0x4200bcea <+322>:   sw      a0,-28(s0) #compute calc stored from a0 to s0, prob 37
   0x4200bcee <+326>:   lw      a1,-28(s0)
   0x4200bcf2 <+330>:   lui     a5,0x3c026
   0x4200bcf6 <+334>:   addi    a0,a5,-760 # 0x3c025d08
   0x4200bcfa <+338>:   jal     ra,0x42010842 <printf> //prints out compute value?
   0x4200bcfe <+342>:   li      a0,100
   0x4200bd02 <+346>:   auipc   ra,0xfe37d
   0x4200bd06 <+350>:   jalr    618(ra) # 0x40388f6c <vTaskDelay>
   0x4200bd0a <+354>:   auipc   ra,0xfe37f
   0x4200bd0e <+358>:   jalr    -926(ra) # 0x4038a96c <__getreent>
   0x4200bd12 <+362>:   mv      a5,a0
   0x4200bd14 <+364>:   lw      a5,8(a5)
   0x4200bd16 <+366>:   mv      a0,a5
   0x4200bd18 <+368>:   jal     ra,0x4200fb36 <fflush>
   0x4200bd1c <+372>:   j       0x4200bcd6 <app_main+302>
   0x4200bd1e <+374>:   lw      ra,44(sp)
   0x4200bd20 <+376>:   lw      s0,40(sp)
   0x4200bd22 <+378>:   addi    sp,sp,48
   0x4200bd24 <+380>:   ret
