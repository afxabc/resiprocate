#include "g723basop_t.h"

DECSTATDEF  DecStat  ;
CODCNGDEF CodCng;
CODSTATDEF  CodStat  ;
DECCNGDEF DecCng;
VADSTATDEF  VadStat ;

/* Global variables */
enum  Wmode    WrkMode = Both ;
enum  Crate    WrkRate = Rate53 ;

int   PackedFrameSize[2] = {
   24 ,
      20
         } ;

         Flag    UseHp = True ;
         Flag    UsePf = True ;
         Flag    UseVx = False ;
         Flag    UsePr = True ;

         char  SignOn[] = "ACL/USH/FT/DSPG ANSI C CODEC ITU LBC Ver 5.00\n" ;

/*___________________________________________________________________________
 |                                                                           |
 | Basics operators.                                                         |
 |___________________________________________________________________________|
*/

/*___________________________________________________________________________
 |                                                                           |
 |   Include-Files                                                           |
 |___________________________________________________________________________|
*/



/*___________________________________________________________________________
 |                                                                           |
 |   Local Functions                                                         |
 |___________________________________________________________________________|
*/
Word16 g723sature(Word32 L_var1);


/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 |___________________________________________________________________________|
*/
Flag g723Overflow =0;
Flag g723Carry =0;


/*___________________________________________________________________________
 |                                                                           |
 |   Functions                                                               |
 |___________________________________________________________________________|
*/


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723sature                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Limit the 32 bit input to the range of a 16 bit word.                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 g723sature(Word32 L_var1)
{
    Word16 var_out;

    if (L_var1 > 0X00007fffL) {
        g723Overflow = 1;
        var_out = MAX_16;
    }
    else {
        if (L_var1 < (Word32)0xffff8000L) {
            g723Overflow = 1;
            var_out = MIN_16;
        }
        else {
            g723Overflow = 0;
            var_out = g723extract_l(L_var1);
        }
    }
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723add                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the addition (var1+var2) with overflow control and saturation;|
 |    the 16 bit result is set at +32767 when overflow occurs or at -32768   |
 |    when underflow occurs.                                                 |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 g723add(Word16 var1,Word16 var2)
{
    Word16 var_out;
    Word32 L_somme;

    L_somme = (Word32) var1 + (Word32) var2;
    var_out = g723sature(L_somme);
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723sub                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the subtraction (var1+var2) with overflow control and satu-   |
 |    ration; the 16 bit result is set at +32767 when overflow occurs or at  |
 |    -32768 when underflow occurs.                                          |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 g723sub(Word16 var1,Word16 var2)
{
    Word16 var_out;
    Word32 L_diff;

    L_diff = (Word32) var1 -  (Word32) var2;
    var_out = g723sature(L_diff);
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : abs_s                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Absolute value of var1; abs_s(-32768) = 32767.                         |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 g723abs_s(Word16 var1)
{
    Word16 var_out;

    if (var1 == (Word16)0X8000 ) {
        var_out = MAX_16;
    }
    else {
        if (var1 < 0) {
            var_out = -var1;
        }
        else {
            var_out = var1;
        }
    }

    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723shl                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 16 bit input var1 left var2 positions.Zero fill|
 |   the var2 LSB of the result. If var2 is negative, arithmetically shift   |
 |   var1 right by -var2 with sign extension. Saturate the result in case of |
 |   underflows or overflows.                                                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 g723shl(Word16 var1,Word16 var2)
{
    Word16 var_out;
    Word32 L_result;

    if (var2 < 0) {
        var_out = g723shr(var1, (Word16)-var2);
    }
    else {
        L_result = (Word32) var1 * ((Word32) 1 << var2);
        if ( ((var2 > 15) && (var1 != 0)) ||
             (L_result != (Word32)((Word16) L_result)) ) {
            g723Overflow = 1;
            var_out = (var1 > 0) ? MAX_16 : MIN_16;
        }
        else {
            var_out = g723extract_l(L_result);
        }
    }

    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : shr                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 16 bit input var1 right var2 positions with    |
 |   sign extension. If var2 is negative, arithmetically shift var1 left by  |
 |   -var2 with sign extension. Saturate the result in case of underflows or |
 |   overflows.                                                              |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 g723shr(Word16 var1,Word16 var2)
{
    Word16 var_out;

    if (var2 < 0) {
        var_out = g723shl(var1, (Word16)-var2);
    }
    else {
        if (var2 >= 15) {
            var_out = (var1 < (Word16)0) ? (Word16)-1 : (Word16)0;
        }
        else {
            if (var1 < 0) {
                var_out = ~(( ~var1) >> var2 );
            }
            else {
                var_out = var1 >> var2;
            }
        }
    }

    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723mult                                                    |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the multiplication of var1 by var2 and gives a 16 bit result  |
 |    which is scaled i.e.:                                                  |
 |             g723mult(var1,var2) = shr((var1 times var2),15) and               |
 |             g723mult(-32768,-32768) = 32767.                                  |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 g723mult(Word16 var1, Word16 var2)
{
    Word16 var_out;
    Word32 L_produit;

    L_produit = (Word32)var1 * (Word32)var2;

    L_produit = (L_produit & (Word32) 0xffff8000L) >> 15;

    if (L_produit & (Word32) 0x00010000L) {
        L_produit |= (Word32) 0xffff0000L;
    }
    var_out = g723sature(L_produit);
    return(var_out);
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_mult                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   L_mult is the 32 bit result of the multiplication of var1 times var2    |
 |   with one shift left i.e.:                                               |
 |        L_mult(var1,var2) = g723shl((var1 times var2),1) and                   |
 |        L_mult(-32768,-32768) = 2147483647.                                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 g723L_mult(Word16 var1,Word16 var2)
{
    Word32 L_var_out;

    L_var_out = (Word32)var1 * (Word32)var2;
    if (L_var_out != (Word32)0x40000000L) {
        L_var_out *= 2L;
    }
    else {
        g723Overflow = 1;
        L_var_out = MAX_32;
    }

    return(L_var_out);
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : negate                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Negate var1 with saturation, saturate in the case where input is -32768:|
 |                negate(var1) = g723sub(0,var1).                                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 g723negate(Word16 var1)
{
    Word16 var_out;

    var_out = (var1 == MIN_16) ? MAX_16 : -var1;
    return(var_out);
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : extract_h                                               |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Return the 16 MSB of L_var1.                                            |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32 ) whose value falls in the |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 g723extract_h(Word32 L_var1)
{
    Word16 var_out;

    var_out = (Word16) (L_var1 >> 16);
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723extract_l                                               |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Return the 16 LSB of L_var1.                                            |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32 ) whose value falls in the |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 g723extract_l(Word32 L_var1)
{
    Word16 var_out;

    var_out = (Word16) L_var1;
    return(var_out);
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723round                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Round the lower 16 bits of the 32 bit input number into its MS 16 bits  |
 |   with saturation. Shift the resulting bits right by 16 and return the 16 |
 |   bit number:                                                             |
 |               g723round(L_var1) = extract_h(g723L_add(L_var1,32768))              |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32 ) whose value falls in the |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 g723round(Word32 L_var1)
{
    Word16 var_out;
    Word32 L_arrondi;

//    L_arrondi = g723L_add(L_var1, (Word32)0x00008000L);
	L_ADD(L_var1, (Word32)0x00008000L, L_arrondi);
    var_out = g723extract_h(L_arrondi);
    return(var_out);
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_mac                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Add the 32 bit    |
 |   result to L_var3 with saturation, return a 32 bit result:               |
 |        g723L_mac(L_var3,var1,var2) = g723L_add(L_var3,(L_mult(var1,var2)).        |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 g723L_mac(Word32 L_var3, Word16 var1, Word16 var2)
{
    Word32 L_var_out;
    Word32 L_produit;
/*
    L_produit = L_mult(var1, var2);
    L_var_out = g723L_add(L_var3, L_produit);*/


	L_MULT(var1, var2, L_produit);
	L_ADD(L_var3, L_produit, L_var_out);

    return(L_var_out);
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_msu                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Subtract the 32   |
 |   bit result to L_var3 with saturation, return a 32 bit result:           |
 |        g723L_msu(L_var3,var1,var2) = g723L_sub(L_var3,(L_mult(var1,var2)).        |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 g723L_msu(Word32 L_var3, Word16 var1, Word16 var2)
{
    Word32 L_var_out;
    Word32 L_produit;

//    L_produit = L_mult(var1, var2);
	L_MULT(var1, var2, L_produit);
    L_var_out = g723L_sub(L_var3, L_produit);

    return(L_var_out);
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_macNs                                                 |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Add the 32 bit    |
 |   result to L_var3 without saturation, return a 32 bit result. Generate   |
 |   carry and overflow values :                                             |
 |        g723L_macNs(L_var3,var1,var2) = g723L_add_c(L_var3,(L_mult(var1,var2)).    |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |                                                                           |
 |   Caution :                                                               |
 |                                                                           |
 |    In some cases the g723Carry flag has to be cleared or set before using op- |
 |    rators which take into account its value.                              |
 |___________________________________________________________________________|
*/

Word32 g723L_macNs(Word32 L_var3, Word16 var1, Word16 var2)
{
    Word32 L_var_out;

//    L_var_out = L_mult(var1, var2);
	L_MULT(var1, var2, L_var_out);
    L_var_out = g723L_add_c(L_var3, L_var_out);
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_msuNs                                                 |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Subtract the 32   |
 |   bit result from L_var3 without saturation, return a 32 bit result. Ge-  |
 |   nerate carry and overflow values :                                      |
 |        g723L_msuNs(L_var3,var1,var2) = g723L_sub_c(L_var3,(L_mult(var1,var2)).    |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |                                                                           |
 |   Caution :                                                               |
 |                                                                           |
 |    In some cases the g723Carry flag has to be cleared or set before using op- |
 |    rators which take into account its value.                              |
 |___________________________________________________________________________|
*/

Word32 g723L_msuNs(Word32 L_var3, Word16 var1, Word16 var2)
{
    Word32 L_var_out;

//    L_var_out = L_mult(var1,var2);
	L_MULT(var1, var2, L_var_out);
    L_var_out = g723L_sub_c(L_var3,L_var_out);
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_add                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   32 bits addition of the two 32 bits variables (L_var1+L_var2) with      |
 |   overflow control and saturation; the result is set at +214783647 when   |
 |   overflow occurs or at -214783648 when underflow occurs.                 |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 g723L_add(Word32 L_var1, Word32 L_var2)
{
    Word32 L_var_out;

    L_var_out = L_var1 + L_var2;

    if (((L_var1 ^ L_var2) & MIN_32) == 0L) {
        if ((L_var_out ^ L_var1) & MIN_32) {
            L_var_out = (L_var1 < 0L) ? MIN_32 : MAX_32;
            g723Overflow = 1;
        }
    }
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_sub                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   32 bits subtraction of the two 32 bits variables (L_var1-L_var2) with   |
 |   overflow control and saturation; the result is set at +214783647 when   |
 |   overflow occurs or at -214783648 when underflow occurs.                 |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 g723L_sub(Word32 L_var1, Word32 L_var2)
{
    Word32 L_var_out;

    L_var_out = L_var1 - L_var2;

    if (((L_var1 ^ L_var2) & MIN_32) != 0L) {
        if ((L_var_out ^ L_var1) & MIN_32) {
            L_var_out = (L_var1 < 0L) ? MIN_32 : MAX_32;
            g723Overflow = 1;
        }
    }
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_add_c                                                 |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Performs 32 bits addition of the two 32 bits variables (L_var1+L_var2+C)|
 |   with carry. No saturation. Generate carry and g723Overflow values. The car- |
 |   ry and overflow values are binary variables which can be tested and as- |
 |   signed values.                                                          |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |                                                                           |
 |   Caution :                                                               |
 |                                                                           |
 |    In some cases the g723Carry flag has to be cleared or set before using op- |
 |    rators which take into account its value.                              |
 |___________________________________________________________________________|
*/
Word32 g723L_add_c(Word32 L_var1, Word32 L_var2)
{
    Word32 L_var_out;
    Word32 L_test;
    Flag carry_int = 0;

    L_var_out = L_var1 + L_var2 + (Word32)g723Carry;

    L_test = L_var1 + L_var2;

    if ((L_var1 > 0L) && (L_var2 > 0L) && (L_test < 0L)) {
        g723Overflow = 1;
        carry_int = 0;
    }
    else {
        if ((L_var1 < 0L) && (L_var2 < 0L) && (L_test > 0L)) {
            g723Overflow = 1;
            carry_int = 1;
        }
        else {
            if (((L_var1 ^ L_var2) < 0L) && (L_test > 0L)) {
                g723Overflow = 0;
                carry_int = 1;
            }
            else {
                g723Overflow = 0;
                carry_int = 0;
            }
        }
    }

    if (g723Carry) {
        if (L_test == MAX_32) {
            g723Overflow = 1;
            g723Carry = carry_int;
        }
        else {
            if (L_test == (Word32) 0xFFFFFFFFL) {
                g723Carry = 1;
            }
            else {
                g723Carry = carry_int;
            }
        }
    }
    else {
        g723Carry = carry_int;
    }

    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_sub_c                                                 |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Performs 32 bits subtraction of the two 32 bits variables with carry    |
 |   (borrow) : L_var1-L_var2-C. No saturation. Generate carry and g723Overflow  |
 |   values. The carry and overflow values are binary variables which can    |
 |   be tested and assigned values.                                          |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |                                                                           |
 |   Caution :                                                               |
 |                                                                           |
 |    In some cases the g723Carry flag has to be cleared or set before using op- |
 |    rators which take into account its value.                              |
 |___________________________________________________________________________|
*/

Word32 g723L_sub_c(Word32 L_var1, Word32 L_var2)
{
    Word32 L_var_out;
    Word32 L_test;
    Flag carry_int = 0;

    if (g723Carry) {
        g723Carry = 0;
        if (L_var2 != MIN_32) {
            L_var_out = g723L_add_c(L_var1,-L_var2);
        }
        else {
            L_var_out = L_var1 - L_var2;
            if (L_var1 > 0L) {
                g723Overflow = 1;
                g723Carry = 0;
            }
        }
    }
    else {
        L_var_out = L_var1 - L_var2 - (Word32)0X00000001L;
        L_test = L_var1 - L_var2;

        if ((L_test < 0L) && (L_var1 > 0L) && (L_var2 < 0L)) {
            g723Overflow = 1;
            carry_int = 0;
        }
        else if ((L_test > 0L) && (L_var1 < 0L) && (L_var2 > 0L)) {
            g723Overflow = 1;
            carry_int = 1;
        }
        else if ((L_test > 0L) && ((L_var1 ^ L_var2) > 0L)) {
            g723Overflow = 0;
            carry_int = 1;
        }


        if (L_test == MIN_32) {
            g723Overflow = 1;
            g723Carry = carry_int;
        }
        else {
            g723Carry = carry_int;
        }
    }

    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_negate                                                |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Negate the 32 bit variable L_var1 with saturation; saturate in the case |
 |   where input is -2147483648 (0x8000 0000).                               |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 g723L_negate(Word32 L_var1)
{
    Word32 L_var_out;

    L_var_out = (L_var1 == MIN_32) ? MAX_32 : -L_var1;
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723mult_r                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Same as g723mult with rounding, i.e.:                                       |
 |     g723mult_r(var1,var2) = shr(((var1*var2) + 16384),15) and                 |
 |     g723mult_r(-32768,-32768) = 32767.                                        |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 g723mult_r(Word16 var1, Word16 var2)
{
    Word16 var_out;
    Word32 L_produit_arr;

    L_produit_arr = (Word32)var1 * (Word32)var2; /* product */
    L_produit_arr += (Word32) 0x00004000L;        /* g723round */
    L_produit_arr &= (Word32) 0xffff8000L;
    L_produit_arr >>= 15;                        /* shift */

    if (L_produit_arr & (Word32) 0x00010000L)  {
                                     /* sign extend when necessary */
        L_produit_arr |= (Word32) 0xffff0000L;
    }

    var_out = g723sature(L_produit_arr);
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_shl                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 32 bit input L_var1 left var2 positions. Zero  |
 |   fill the var2 LSB of the result. If var2 is negative, L_var1 right by   |
 |   -var2 arithmetically shift with sign extension. Saturate the result in  |
 |   case of underflows or overflows.                                        |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 g723L_shl(Word32 L_var1, Word16 var2)
{
    Word32 L_var_out = 0L;

    if (var2 <= (Word16)0) {
        L_var_out = g723L_shr(L_var1, (Word16)-var2);
    }
    else {
        for(; var2 > (Word16)0; var2--) {
            if (L_var1 > (Word32) 0X3fffffffL) {
                g723Overflow = 1;
                L_var_out = MAX_32;
                break;
            }
            else {
                if (L_var1 < (Word32) 0xc0000000L) {
                    g723Overflow = 1;
                    L_var_out = MIN_32;
                    break;
                }
            }
            L_var1 *= 2L;
            L_var_out = L_var1;
        }
    }
    return(L_var_out);
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_shr                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 32 bit input L_var1 right var2 positions with  |
 |   sign extension. If var2 is negative, arithmetically shift L_var1 left   |
 |   by -var2 and zero fill the var2 LSB of the result. Saturate the result  |
 |   in case of underflows or overflows.                                     |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 g723L_shr(Word32 L_var1, Word16 var2)
{
    Word32 L_var_out;

    if (var2 < (Word16)0) {
        L_var_out = g723L_shl(L_var1, (Word16)-var2);
    }
    else {
        if (var2 >= (Word16)31) {
            L_var_out = (L_var1 < 0L) ? -1L : 0L;
        }
        else {
            if (L_var1< 0L)  {
                L_var_out = ~((~L_var1) >> var2);
            }
            else {
                L_var_out = L_var1 >> var2;
            }
        }
    }
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723shr_r                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Same as shr(var1,var2) but with rounding. Saturate the result in case of|
 |   underflows or overflows :                                               |
 |    If var2 is greater than zero :                                         |
 |       g723shr_r(var1,var2) = shr(g723add(var1,2**(var2-1)),var2)                  |
 |    If var2 is less than zero :                                            |
 |       g723shr_r(var1,var2) = shr(var1,var2).                                  |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 g723shr_r(Word16 var1, Word16 var2)
{
    Word16 var_out;

    if (var2 > (Word16)15) {
        var_out = (Word16)0;
    }
    else {
        var_out = g723shr(var1,var2);

        if (var2 > (Word16)0) {
            if ((var1 & ((Word16)1 << (var2-(Word16)1))) != (Word16)0) {
                var_out++;
            }
        }
    }
    return(var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723mac_r                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Add the 32 bit    |
 |   result to L_var3 with saturation. Round the LS 16 bits of the result    |
 |   into the MS 16 bits with saturation and shift the result right by 16.   |
 |   Return a 16 bit result.                                                 |
 |            g723mac_r(L_var3,var1,var2) = g723round(g723L_mac(Lvar3,var1,var2))        |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 8000 <= L_var_out <= 0x0000 7fff.              |
 |___________________________________________________________________________|
*/

Word16 g723mac_r(Word32 L_var3, Word16 var1, Word16 var2)
{
    Word16 var_out;

    L_var3 = g723L_mac(L_var3, var1, var2);
//    L_var3 = g723L_add(L_var3, (Word32) 0x00008000L);
	L_ADD(L_var3, (Word32) 0x00008000L, L_var3);
    var_out = g723extract_h(L_var3);
    return(var_out);
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723msu_r                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Subtract the 32   |
 |   bit result to L_var3 with saturation. Round the LS 16 bits of the res-  |
 |   ult into the MS 16 bits with saturation and shift the result right by   |
 |   16. Return a 16 bit result.                                             |
 |            g723msu_r(L_var3,var1,var2) = g723round(g723L_msu(Lvar3,var1,var2))        |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 8000 <= L_var_out <= 0x0000 7fff.              |
 |___________________________________________________________________________|
*/

Word16 g723msu_r(Word32 L_var3, Word16 var1, Word16 var2)
{
    Word16 var_out;

    L_var3 = g723L_msu(L_var3,var1,var2);
//    L_var3 = g723L_add(L_var3, (Word32) 0x00008000L);
	L_ADD(L_var3, (Word32) 0x00008000L, L_var3);
    var_out = g723extract_h(L_var3);
    return(var_out);
}



/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_deposit_h                                             |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Deposit the 16 bit var1 into the 16 MS bits of the 32 bit output. The   |
 |   16 LS bits of the output are zeroed.                                    |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var_out <= 0x7fff 0000.                |
 |___________________________________________________________________________|
*/

Word32 g723L_deposit_h(Word16 var1)
{
    Word32 L_var_out;

    L_var_out = (Word32) var1 << 16;
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_deposit_l                                             |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Deposit the 16 bit var1 into the 16 LS bits of the 32 bit output. The   |
 |   16 MS bits of the output are sign extended.                             |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0xFFFF 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word32 g723L_deposit_l(Word16 var1)
{
    Word32 L_var_out;

    L_var_out = (Word32) var1;
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_shr_r                                                 |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Same as g723L_shr(L_var1,var2)but with rounding. Saturate the result in case|
 |   of underflows or overflows :                                            |
 |    If var2 is greater than zero :                                         |
 |       g723L_shr_r(var1,var2) = g723L_shr(g723L_add(L_var1,2**(var2-1)),var2)          |
 |    If var2 is less than zero :                                            |
 |       g723L_shr_r(var1,var2) = g723L_shr(L_var1,var2).                            |
 |                                                                           |
 |   Complexity weight : 3                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var_out <= 0x7fff ffff.                |
 |___________________________________________________________________________|
*/

Word32 g723L_shr_r(Word32 L_var1,Word16 var2)
{
    Word32 L_var_out;

    if (var2 > (Word16)31) {
        L_var_out = 0L;
    }
    else {
        L_var_out = g723L_shr(L_var1,var2);
        if (var2 > (Word16)0) {
            if ( (L_var1 & ( 1L << (Word16)(var2-(Word16)1) )) != 0L)  {
                L_var_out++;
            }
        }
    }
    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_abs                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Absolute value of L_var1; Saturate in case where the input is          |
 |                                                               -214783648  |
 |                                                                           |
 |   Complexity weight : 3                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x0000 0000 <= var_out <= 0x7fff ffff.                |
 |___________________________________________________________________________|
*/

Word32 g723L_abs(Word32 L_var1)
{
    Word32 L_var_out;

    if (L_var1 == MIN_32) {
        L_var_out = MAX_32;
    }
    else {
        if (L_var1 < 0L) {
            L_var_out = -L_var1;
        }
        else {
            L_var_out = L_var1;
        }
    }

    return(L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723L_sat                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    32 bit L_var1 is set to 2147833647 if an overflow occured or to        |
 |    -214783648 if an underflow occured on the most recent g723L_add_c, g723L_sub_c,|
 |    g723L_macNs or LmsuNs operations. The carry and overflow values are binary |
 |    values which can be tested and assigned values.                        |
 |                                                                           |
 |   Complexity weight : 4                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var_out <= 0x7fff ffff.                |
 |___________________________________________________________________________|
*/

Word32 g723L_sat (Word32 L_var1)
{
    Word32 L_var_out;


    L_var_out = L_var1;

    if (g723Overflow) {

        if (g723Carry) {
            L_var_out = MIN_32;
        }
        else {
            L_var_out = MAX_32;
        }

        g723Carry = 0;
        g723Overflow = 0;
    }

    return(L_var_out);
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723norm_s                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces the number of left shift needed to normalize the 16 bit varia- |
 |   ble var1 for positive values on the interval with minimum of 16384 and  |
 |   maximum of 32767, and for negative values on the interval with minimum  |
 |   of -32768 and maximum of -16384; in order to normalize the result, the  |
 |   following operation must be done :                                      |
 |                    norm_var1 = g723shl(var1,g723norm_s(var1)).                    |
 |                                                                           |
 |   Complexity weight : 15                                                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 000f.                |
 |___________________________________________________________________________|
*/

Word16 g723norm_s(Word16 var1)
{
    Word16 var_out;

    if (var1 == (Word16)0) {
        var_out = (Word16)0;
    }
    else {
        if (var1 == (Word16) 0xffff) {
            var_out = (Word16)15;
        }
        else {
            if (var1 < (Word16)0) {
                var1 = ~var1;
            }

            for(var_out = (Word16)0; var1 < (Word16)0x4000; var_out++) {
                var1 <<= 1;
            }
        }
    }

    return(var_out);
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723div_s                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces a result which is the fractional integer division of var1 by  |
 |   var2; var1 and var2 must be positive and var2 must be greater or equal  |
 |   to var1; the result is positive (leading bit equal to 0) and truncated  |
 |   to 16 bits.                                                             |
 |   If var1 = var2 then div(var1,var2) = 32767.                             |
 |                                                                           |
 |   Complexity weight : 18                                                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 0000 <= var1 <= var2 and var2 != 0.            |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : var1 <= var2 <= 0x0000 7fff and var2 != 0.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 7fff.                |
 |             It's a Q15 value (point between b15 and b14).                 |
 |___________________________________________________________________________|
*/

Word16 g723div_s(Word16 var1, Word16 var2)
{
    Word16 var_out = (Word16)0;
    Word16 iteration;
    Word32 L_num;
    Word32 L_denom;

    if ((var1 > var2) || (var1 < (Word16)0) || (var2 < (Word16)0)) {
/*        printf("Division Error\n");*/
        return 0;
    }

    if (var2 == (Word16)0) {
/*        printf("Division by 0, Fatal error \n");*/
        return 0;
    }

    if (var1 == (Word16)0) {
        var_out = (Word16)0;
    }
    else {
        if (var1 == var2) {
            var_out = MAX_16;
        }
        else {
            L_num = g723L_deposit_l(var1);
            L_denom = g723L_deposit_l(var2);

            for(iteration=(Word16)0; iteration< (Word16)15;iteration++) {
                var_out <<=1;
                L_num <<= 1;

                if (L_num >= L_denom) {
                    L_num = g723L_sub(L_num,L_denom);
                    var_out = g723add(var_out, (Word16)1);
                }
            }
        }
    }

    return(var_out);
}


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : g723norm_l                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces the number of left shift needed to normalize the 32 bit varia- |
 |   ble l_var1 for positive values on the interval with minimum of          |
 |   1073741824 and maximum of 2147483647, and for negative values on the in-|
 |   terval with minimum of -2147483648 and maximum of -1073741824; in order |
 |   to normalize the result, the following operation must be done :         |
 |                   norm_L_var1 = g723L_shl(L_var1,g723norm_l(L_var1)).             |
 |                                                                           |
 |   Complexity weight : 30                                                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 001f.                |
 |___________________________________________________________________________|
*/

Word16 g723norm_l(Word32 L_var1)
{
    Word16 var_out;

    if (L_var1 == 0L) {
        var_out = (Word16)0;
    }
    else {
        if (L_var1 == (Word32)0xffffffffL) {
            var_out = (Word16)31;
        }
        else {
            if (L_var1 < 0L) {
                L_var1 = ~L_var1;
            }

            for(var_out = (Word16)0;L_var1 < (Word32)0x40000000L;var_out++) {
                L_var1 <<= 1L;
            }
        }
    }

    return(var_out);
}
/*
   Additional operators
*/
Word32   L_mls( Word32 Lv, Word16 v )
{
   Word32   Temp  ;

   Temp = Lv & (Word32) 0x0000ffff ;
   Temp = Temp * (Word32) v ;
   Temp = g723L_shr( Temp, (Word16) 15 ) ;
   Temp = g723L_mac( Temp, v, g723extract_h(Lv) ) ;

   return Temp ;
}
/*__________________________________________________________________________
|                                                                           |
|   Function Name : div_l                                                   |
|                                                                           |
|   Purpose :                                                               |
|                                                                           |
|   Produces a result which is the fractional integer division of L_var1 by|
|   var2; L_var1 and var2 must be positive and var2 << 16 must be greater or|
|   equal to L_var1; the result is positive (leading bit equal to 0) and    |
|   truncated to 16 bits.                                                   |
|   If L_var1 == var2 << 16 then div_l(L_var1,var2) = 32767.                |
|                                                                           |
|   Complexity weight : 20                                                  |
|                                                                           |
|   Inputs :                                                                |
|                                                                           |
|    L_var1                                                                 |
|             32 bit long signed integer (Word32) whose value falls in the  |
|             range : 0x0000 0000 <= var1 <= (var2 << 16)  and var2 != 0.   |
|             L_var1 must be considered as a Q.31 value                     |
|                                                                           |
|    var2                                                                   |
|             16 bit short signed integer (Word16) whose value falls in the |
|             range : var1 <= (var2<< 16) <= 0x7fff0000 and var2 != 0.      |
|             var2 must be considered as a Q.15 value                       |
|                                                                           |
|   Outputs :                                                               |
|                                                                           |
|    none                                                                   |
|                                                                           |
|   Return Value :                                                          |
|                                                                           |
|    var_out                                                                |
|             16 bit short signed integer (Word16) whose value falls in the |
|             range : 0x0000 0000 <= var_out <= 0x0000 7fff.                |
|             It's a Q15 value (point between b15 and b14).                 |
|___________________________________________________________________________|
*/


Word16 div_l( Word32  L_num, Word16 den ){

    Word16   var_out = (Word16)0;
    Word32   L_den;
    Word16   iteration;

    if ( den == (Word16) 0 ) {
/*        printf("Division by 0 in div_l, Fatal error \n");*/
        return(0);
    }

    if ( (L_num < (Word32) 0) || (den < (Word16) 0) ) {
/*        printf("Division Error in div_l, Fatal error \n");*/
        return(0);
    }

    L_den = g723L_deposit_h( den ) ;

    if ( L_num >= L_den ){
        return MAX_16 ;
    }
    else {
        L_num = g723L_shr(L_num, (Word16)1) ;
        L_den = g723L_shr(L_den, (Word16)1);
        for(iteration=(Word16)0; iteration< (Word16)15;iteration++) {
            var_out = g723shl( var_out, (Word16)1);
            L_num   = g723L_shl( L_num, (Word16)1);
            if (L_num >= L_den) {
                L_num = g723L_sub(L_num,L_den);
                var_out = g723add(var_out, (Word16)1);
            }
        }

        return var_out;
    }
}

/*---------------------------------------------------------------------------*
 *  Function  i_mult()                                                        *
 *  ~~~~~~~~~~~~~~~~~                                                        *
 * Integer multiplication.                                                   *
 *--------------------------------------------------------------------------*/
Word16 i_mult(Word16 a, Word16 b)
{
   return  a*b;
}


