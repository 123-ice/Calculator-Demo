// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

//-----------------------------------------------------------------------------
//  Package Title  ratpak
//  File           num.c
//  Copyright      (C) 1995-97 Microsoft
//  Date           01-16-95
//
//
//  Description
//
//     Contains number routines for add, mul, div, rem and other support
//  and longs.
//
//  Special Information
//
//
//-----------------------------------------------------------------------------
#include "pch.h"
#include "ratpak.h"

using namespace std;

//----------------------------------------------------------------------------
//
//    FUNCTION: addnum
//
//    ARGUMENTS: pointer to a number a second number, and the
//               radix.
//
//    RETURN: None, changes first pointer.
//
//    DESCRIPTION: Does the number equivalent of *pa += b.
//    Assumes radix is the base of both numbers.
//
//    ALGORITHM: Adds each digit from least significant to most
//    significant.
//
//
//----------------------------------------------------------------------------

void _addnum( NUMBER *pa, NUMBER b, uint32_t radix);

void __inline addnum( NUMBER *pa, NUMBER b, uint32_t radix)

{
    if ( b.cdigit > 1 || b.mant[0] != 0 )
        {    // If b is zero we are done.
        if ( (*pa).cdigit > 1 || (*pa).mant[0] != 0 )
            { // pa and b are both nonzero.
            _addnum( pa, b, radix);
            }
        else
            { // if pa is zero and b isn't just copy b.
            DUPNUM(*pa,b);
            }
        }
}

void _addnum( NUMBER *pa, NUMBER b, uint32_t radix)

{
    NUMBER c;              // c will contain the result.
    NUMBER a;              // a is the dereferenced number pointer from *pa
    vector<MANTTYPE>::iterator pcha; // pcha is an iterator pointing to the mantissa of a.
    vector<MANTTYPE>::iterator pchb; // pchb is an iterator pointing to the mantissa of b.
    vector<MANTTYPE>::iterator pchc; // pchc is an iterator pointing to the mantissa of c.
    int32_t cdigits;       // cdigits is the max count of the digits results
                        // used as a counter.
    int32_t mexp;          // mexp is the exponent of the result.
    MANTTYPE  da;       // da is a single 'digit' after possible padding.
    MANTTYPE  db;       // db is a single 'digit' after possible padding.
    MANTTYPE  cy=0;     // cy is the value of a carry after adding two 'digits'
    int32_t  fcompla = 0;  // fcompla is a flag to signal a is negative.
    int32_t  fcomplb = 0;  // fcomplb is a flag to signal b is negative.

    a=*pa;


    // Calculate the overlap of the numbers after alignment, this includes
    // necessary padding 0's
    cdigits = max( a.cdigit+a.exp, b.cdigit+b.exp ) -
            min( a.exp, b.exp );

    createnum( c, cdigits + 1 );
    c.exp = min( a.exp, b.exp );
    mexp = c.exp;
    c.cdigit = cdigits;
    pcha = a.mant.begin();
    pchb = b.mant.begin();
    pchc = c.mant.begin();

    // Figure out the sign of the numbers
    if ( a.sign != b.sign )
        {
        cy = 1;
        fcompla = ( a.sign == -1 );
        fcomplb = ( b.sign == -1 );
        }

    // Loop over all the digits, real and 0 padded. Here we know a and b are
    // aligned
    for ( ;cdigits > 0; cdigits--, mexp++ )
        {

        // Get digit from a, taking padding into account.
        da = ( ( ( mexp >= a.exp ) && ( cdigits + a.exp - c.exp >
                    (c.cdigit - a.cdigit) ) ) ?
                    *pcha++ : 0 );
        // Get digit from b, taking padding into account.
        db = ( ( ( mexp >= b.exp ) && ( cdigits + b.exp - c.exp >
                    (c.cdigit - b.cdigit) ) ) ?
                    *pchb++ : 0 );

        // Handle complementing for a and b digit. Might be a better way, but
        // haven't found it yet.
        if ( fcompla )
            {
            da = (MANTTYPE)(radix) - 1 - da;
            }
        if ( fcomplb )
            {
            db = (MANTTYPE)(radix) - 1 - db;
            }

        // Update carry as necessary
        cy = da + db + cy;
        *pchc++ = (MANTTYPE)(cy % (MANTTYPE)radix);
        cy /= (MANTTYPE)radix;
        }

    // Handle carry from last sum as extra digit
    if ( cy && !(fcompla || fcomplb) )
        {
        *pchc++ = cy;
        c.cdigit++;
        }

    // Compute sign of result
    if ( !(fcompla || fcomplb) )
        {
        c.sign = a.sign;
        }
    else
        {
        if ( cy )
            {
            c.sign = 1;
            }
        else
            {
            // In this particular case an overflow or underflow has occurred
            // and all the digits need to be complemented, at one time an
            // attempt to handle this above was made, it turned out to be much
            // slower on average.
            c.sign = -1;
            cy = 1;
            for ( ( cdigits = c.cdigit ), (pchc = c.mant.begin());
                cdigits > 0;
                cdigits-- )
                {
                cy = (MANTTYPE)radix - (MANTTYPE)1 - *pchc + cy;
                *pchc++ = (MANTTYPE)( cy % (MANTTYPE)radix);
                cy /= (MANTTYPE)radix;
                }
            }
        }

    // Remove leading zeros, remember digits are in order of
    // increasing significance. i.e. 100 would be 0,0,1
    while ( c.cdigit > 1 && *(--pchc) == 0 )
        {
        c.cdigit--;
        }
    *pa=c;
}

//----------------------------------------------------------------------------
//
//    FUNCTION: mulnum
//
//    ARGUMENTS: pointer to a number a second number, and the
//               radix.
//
//    RETURN: None, changes first pointer.
//
//    DESCRIPTION: Does the number equivalent of *pa *= b.
//    Assumes radix is the radix of both numbers.  This algorithm is the
//    same one you learned in grade school.
//
//----------------------------------------------------------------------------

void _mulnum( NUMBER *pa, NUMBER b, uint32_t radix);

void __inline mulnum( NUMBER *pa, NUMBER b, uint32_t radix)

{
    if ( b.cdigit > 1 || b.mant[0] != 1 || b.exp != 0 )
        {    // If b is one we don't multiply exactly.
        if ( (*pa).cdigit > 1 || (*pa).mant[0] != 1 || (*pa).exp != 0 )
            { // pa and b are both non-one.
            _mulnum( pa, b, radix);
            }
        else
            { // if pa is one and b isn't just copy b, and adjust the sign.
            int32_t sign = (*pa).sign;
            DUPNUM(*pa,b);
            (*pa).sign *= sign;
            }
        }
    else
        {    // But we do have to set the sign.
        (*pa).sign *= b.sign;
        }
}

void _mulnum( NUMBER *pa, NUMBER b, uint32_t radix)

{
    NUMBER c;                    // c will contain the result.
    NUMBER a;                    // a is the dereferenced number pointer from *pa
    vector<MANTTYPE>::iterator pcha;       // pcha is an iterator pointing to the mantissa of a.
    vector<MANTTYPE>::iterator pchb;       // pchb is an iterator pointing to the mantissa of b.
    vector<MANTTYPE>::iterator pchc;       // pchc is an iterator pointing to the mantissa of c.
    vector<MANTTYPE>::iterator pchcoffset; // pchcoffset, is the anchor location of the next
                            // single digit multiply partial result.
    int32_t iadigit = 0;       // Index of digit being used in the first number.
    int32_t ibdigit = 0;       // Index of digit being used in the second number.
    MANTTYPE  da = 0;       // da is the digit from the fist number.
    TWO_MANTTYPE  cy = 0;   // cy is the carry resulting from the addition of
                            // a multiplied row into the result.
    TWO_MANTTYPE  mcy = 0;  // mcy is the resultant from a single
                            // multiply, AND the carry of that multiply.
    int32_t  icdigit = 0;      // Index of digit being calculated in final result.

    a=*pa;
    ibdigit = a.cdigit + b.cdigit - 1;
    createnum( c,  ibdigit + 1 );
    c.cdigit = ibdigit;
    c.sign = a.sign * b.sign;

    c.exp = a.exp + b.exp;
    pcha = a.mant.begin();
    pchcoffset = c.mant.begin();

    for (  iadigit = a.cdigit; iadigit > 0; iadigit-- )
        {
        da =  *pcha++;
        pchb = b.mant.begin();

        // Shift pchc, and pchcoffset, one for each digit
        pchc = pchcoffset++;

        for ( ibdigit = b.cdigit; ibdigit > 0; ibdigit-- )
            {
            cy = 0;
            mcy = (TWO_MANTTYPE)da * *pchb;
            if ( mcy )
                {
                icdigit = 0;
                if ( ibdigit == 1 && iadigit == 1 )
                    {
                    c.cdigit++;
                    }
                }
            // If result is nonzero, or while result of carry is nonzero...
            while ( mcy || cy )
                {

                // update carry from addition(s) and multiply.
                cy += (TWO_MANTTYPE)pchc[icdigit]+(mcy%(TWO_MANTTYPE)radix);

                // update result digit from
                pchc[icdigit++]=(MANTTYPE)(cy%(TWO_MANTTYPE)radix);

                // update carries from
                mcy /= (TWO_MANTTYPE)radix;
                cy /= (TWO_MANTTYPE)radix;
                }

            pchb++;
            pchc++;
            }
        }

    // prevent different kinds of zeros, by stripping leading duplicate zeros.
    // digits are in order of increasing significance.
    while ( c.cdigit > 1 && c.mant[c.cdigit-1] == 0 )
        {
        c.cdigit--;
        }

    *pa=c;
}


//----------------------------------------------------------------------------
//
//    FUNCTION: remnum
//
//    ARGUMENTS: pointer to a number a second number, and the
//               radix.
//
//    RETURN: None, changes first pointer.
//
//    DESCRIPTION: Does the number equivalent of *pa %= b.
//            Repeatedly subtracts off powers of 2 of b until *pa < b.
//
//
//----------------------------------------------------------------------------

void remnum( NUMBER *pa, NUMBER b, uint32_t radix)

{
    optional<NUMBER> tmp;     // tmp is the working remainder.
    optional<NUMBER> lasttmp; // lasttmp is the last remainder which worked.

    // Once *pa is less than b, *pa is the remainder.
    while ( !lessnum( *pa, b ) )
        {
        if ( lessnum( *tmp, *pa ) )
            {
            // Start off close to the right answer for subtraction.
            tmp->exp = (*pa).cdigit+(*pa).exp - tmp->cdigit;
            if ( MSD(*pa) <= MSD(*tmp) )
                {
                // Don't take the chance that the numbers are equal.
                tmp->exp--;
                }
            }

        lasttmp=i32tonum( 0, radix);

        while ( lessnum( *tmp, *pa ) )
            {
            addnum( &*tmp, *tmp, radix);
            }

        if ( lessnum( *pa, *tmp ) )
            {
            // too far, back up...
            tmp=lasttmp;
            lasttmp= optional<NUMBER>();
            }

        // Subtract the working remainder from the remainder holder.
        tmp->sign = -1*(*pa).sign;
        addnum( pa, *tmp, radix);


        }
}


//---------------------------------------------------------------------------
//
//    FUNCTION: divnum
//
//    ARGUMENTS: pointer to a number a second number, and the
//               radix.
//
//    RETURN: None, changes first pointer.
//
//    DESCRIPTION: Does the number equivalent of *pa /= b.
//    Assumes radix is the radix of both numbers.
//
//---------------------------------------------------------------------------

void _divnum( NUMBER *pa, NUMBER b, uint32_t radix, int32_t precision);

void __inline divnum( NUMBER *pa, NUMBER b, uint32_t radix, int32_t precision)

{
    if ( b.cdigit > 1 || b.mant[0] != 1 || b.exp != 0 )
        {
        // b is not one
        _divnum( pa, b, radix, precision);
        }
    else
        {    // But we do have to set the sign.
        (*pa).sign *= b.sign;
        }
}

void _divnum( NUMBER *pa, NUMBER b, uint32_t radix, int32_t precision)
{
    NUMBER a = *pa;
    int32_t thismax = precision + 2;
    if (thismax < a.cdigit)
    {
        thismax = a.cdigit;
    }

    if (thismax < b.cdigit)
    {
        thismax = b.cdigit;
    }

    NUMBER c;
    createnum(c, thismax + 1);
    c.exp = (a.cdigit + a.exp) - (b.cdigit + b.exp) + 1;
    c.sign = a.sign * b.sign;

    vector<MANTTYPE>::iterator ptrc = c.mant.begin() + thismax;
    NUMBER rem;
    NUMBER tmp;
    DUPNUM(rem, a);
    DUPNUM(tmp, b);
    tmp.sign = a.sign;
    rem.exp = b.cdigit + b.exp - rem.cdigit;

    // Build a table of multiplications of the divisor, this is quicker for
    // more than radix 'digits'
    list<NUMBER> numberList{ i32tonum(0L, radix) };
    for (uint32_t i = 1; i < radix; i++)
    {
        NUMBER newValue;
        DUPNUM(newValue, numberList.front());
        addnum(&newValue, tmp, radix);

        numberList.emplace_front(newValue);
    }

    int32_t digit;
    int32_t cdigits = 0;
    while (cdigits++ < thismax && !zernum(rem))
    {
        digit = radix - 1;
        NUMBER multiple;
        for (const auto& num : numberList)
        {
            if (!lessnum(rem, num) || !--digit)
            {
                multiple = num;
                break;
            }
        }

        if (digit)
        {
            multiple.sign *= -1;
            addnum(&rem, multiple, radix);
            multiple.sign *= -1;
        }
        rem.exp++;
        *ptrc-- = (MANTTYPE)digit;
    }
    cdigits--;

    if (c.mant.begin() != ++ptrc)
    {
        copy(ptrc, ptrc + cdigits, c.mant.begin());
    }

    // Cleanup table structure
    for (auto& num : numberList)
    {
    }

    if (!cdigits)
    {
        c.cdigit = 1;
        c.exp = 0;
    }
    else
    {
        c.cdigit = cdigits;
        c.exp -= cdigits;
        while (c.cdigit > 1 && c.mant[c.cdigit - 1] == 0)
        {
            c.cdigit--;
        }
    }

    *pa = c;
}


//---------------------------------------------------------------------------
//
//    FUNCTION: equnum
//
//    ARGUMENTS: two numbers.
//
//    RETURN: Boolean
//
//    DESCRIPTION: Does the number equivalent of ( a == b )
//    Only assumes that a and b are the same radix.
//
//---------------------------------------------------------------------------

bool equnum( NUMBER a, NUMBER b )

{
    int32_t diff;
    int32_t ia;
    int32_t ib;
    int32_t cdigits;
    int32_t ccdigits;
    MANTTYPE  da;
    MANTTYPE  db;

    diff = ( a.cdigit + a.exp ) - ( b.cdigit + b.exp );
    if ( diff < 0 )
        {
        // If the exponents are different, these are different numbers.
        return false;
        }
    else
        {
        if ( diff > 0 )
            {
            // If the exponents are different, these are different numbers.
            return false;
            }
        else
            {
            // OK the exponents match.
            ia = a.cdigit - 1;
            ib = b.cdigit - 1;
            cdigits = max( a.cdigit, b.cdigit );
            ccdigits = cdigits;

            // Loop over all digits until we run out of digits or there is a
            // difference in the digits.
            for ( ;cdigits > 0; cdigits-- )
                {
                da = ( (cdigits > (ccdigits - a.cdigit) ) ?
                    a.mant[ia--] : 0 );
                db = ( (cdigits > (ccdigits - b.cdigit) ) ?
                    b.mant[ib--] : 0 );
                if ( da != db )
                    {
                    return false;
                    }
                }

            // In this case, they are equal.
            return true;
            }
        }
}

//---------------------------------------------------------------------------
//
//    FUNCTION: lessnum
//
//    ARGUMENTS: two numbers.
//
//    RETURN: Boolean
//
//    DESCRIPTION: Does the number equivalent of ( abs(a) < abs(b) )
//    Only assumes that a and b are the same radix, WARNING THIS IS AN.
//    UNSIGNED COMPARE!
//
//---------------------------------------------------------------------------

bool lessnum( NUMBER a, NUMBER b )

{
    int32_t diff;
    int32_t ia;
    int32_t ib;
    int32_t cdigits;
    int32_t ccdigits;
    MANTTYPE  da;
    MANTTYPE  db;


    diff = ( a.cdigit + a.exp ) - ( b.cdigit + b.exp );
    if ( diff < 0 )
        {
        // The exponent of a is less than b
        return true;
        }
    else
        {
        if ( diff > 0 )
            {
            return false;
            }
        else
            {
            ia = a.cdigit - 1;
            ib = b.cdigit - 1;
            cdigits = max( a.cdigit, b.cdigit );
            ccdigits = cdigits;
            for ( ;cdigits > 0; cdigits-- )
                {
                da = ( (cdigits > (ccdigits - a.cdigit) ) ?
                    a.mant[ia--] : 0 );
                db = ( (cdigits > (ccdigits - b.cdigit) ) ?
                    b.mant[ib--] : 0 );
                diff = da-db;
                if ( diff )
                    {
                    return( diff < 0 );
                    }
                }
            // In this case, they are equal.
            return false;
            }
        }
}

//----------------------------------------------------------------------------
//
//    FUNCTION: zernum
//
//    ARGUMENTS: number
//
//    RETURN: Boolean
//
//    DESCRIPTION: Does the number equivalent of ( !a )
//
//----------------------------------------------------------------------------

bool zernum( NUMBER a )

{
    int32_t length = a.cdigit;
    int32_t index = 0;
    
    // loop over all the digits until you find a nonzero or until you run
    // out of digits
    while ( length-- > 0 )
        {
        if ( a.mant[index++] )
            {
            // One of the digits isn't zero, therefore the number isn't zero
            return false;
            }
        }
    // All of the digits are zero, therefore the number is zero
    return true;
}
