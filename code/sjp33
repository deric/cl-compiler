

  1: Program
  2:   Vars
  3:     S Array[2] of Struct
  4:                     X Int
  5:                     A Array [10] Of Int
  6:                   EndStruct
  7:     X Int
  8:   EndVars
  9:   Function F() Return Struct X Int A Array [10] Of Int EndStruct
 10:     X:=X+1
 11:     Return S[X]
 12:   EndFunction
 13:   Function G(Val A0 Array [10] Of Int,
 14:              Val A1 Array [10] Of Int) Return Int
 15:     Return A0[0]+A1[1]
 16:   EndFunction
 17: 
 18:   X:=-1
 19:   S[0].A[0]:=3
 20:   S[1].A[1]:=5
 21:   X:=G(F().A,F().A)
 22:   WriteLn(X)
 23: EndProgram
 24: 


program
    \__list
    |      \__ident(S)
    |      |        \__array
    |      |                \__intconst(2)
    |      |                \__struct
    |      |                         \__ident(X)
    |      |                         |        \__int
    |      |                         \__ident(A)
    |      |                                  \__array
    |      |                                          \__intconst(10)
    |      |                                          \__int
    |      \__ident(X)
    |               \__int
    \__list
    |      \__function
    |      |          \__ident(F)
    |      |          |        \__list
    |      |          |        \__struct
    |      |          |                 \__ident(X)
    |      |          |                 |        \__int
    |      |          |                 \__ident(A)
    |      |          |                          \__array
    |      |          |                                  \__intconst(10)
    |      |          |                                  \__int
    |      |          \__list
    |      |          \__list
    |      |          \__list
    |      |          |      \__:=
    |      |          |           \__ident(X)
    |      |          |           \__+
    |      |          |               \__ident(X)
    |      |          |               \__intconst(1)
    |      |          \__[
    |      |              \__ident(S)
    |      |              \__ident(X)
    |      \__function
    |                 \__ident(G)
    |                 |        \__list
    |                 |        |      \__val
    |                 |        |      |     \__ident(A0)
    |                 |        |      |     \__array
    |                 |        |      |             \__intconst(10)
    |                 |        |      |             \__int
    |                 |        |      \__val
    |                 |        |            \__ident(A1)
    |                 |        |            \__array
    |                 |        |                    \__intconst(10)
    |                 |        |                    \__int
    |                 |        \__int
    |                 \__list
    |                 \__list
    |                 \__list
    |                 \__+
    |                     \__[
    |                     |   \__ident(A0)
    |                     |   \__intconst(0)
    |                     \__[
    |                         \__ident(A1)
    |                         \__intconst(1)
    \__list
           \__:=
           |    \__ident(X)
           |    \__-
           |        \__intconst(1)
           \__:=
           |    \__[
           |    |   \__.
           |    |   |   \__[
           |    |   |   |   \__ident(S)
           |    |   |   |   \__intconst(0)
           |    |   |   \__ident(A)
           |    |   \__intconst(0)
           |    \__intconst(3)
           \__:=
           |    \__[
           |    |   \__.
           |    |   |   \__[
           |    |   |   |   \__ident(S)
           |    |   |   |   \__intconst(1)
           |    |   |   \__ident(A)
           |    |   \__intconst(1)
           |    \__intconst(5)
           \__:=
           |    \__ident(X)
           |    \__(
           |        \__ident(G)
           |        \__list
           |               \__.
           |               |   \__(
           |               |   |   \__ident(F)
           |               |   |   \__list
           |               |   \__ident(A)
           |               \__.
           |                   \__(
           |                   |   \__ident(F)
           |                   |   \__list
           |                   \__ident(A)
           \__writeln
                     \__ident(X)


Type Checking:

Generating code:
program
  parameters
    static_link
  endparameters

  variables
    _S 88
    _X 4
    aux_space 88
  endvariables

    aload _X t0
    iload 1 t1
    mini t1 t1
    stor t1 t0
    aload _S t0
    iload 0 t1
    muli t1 44 t1
    addi t0 t1 t0
    addi t0 4 t0
    iload 0 t1
    muli t1 4 t1
    addi t0 t1 t0
    iload 3 t1
    stor t1 t0
    aload _S t0
    iload 1 t1
    muli t1 44 t1
    addi t0 t1 t0
    addi t0 4 t0
    iload 1 t1
    muli t1 4 t1
    addi t0 t1 t0
    iload 5 t1
    stor t1 t0
    aload _X t0
    pushparam 0
    aload aux_space t1
    addi t1 0 t1
    pushparam t1
    aload static_link t2
    pushparam t2
    call program_F
    killparam
    killparam
    addi t1 4 t1
    pushparam t1
    aload aux_space t1
    addi t1 44 t1
    pushparam t1
    aload static_link t2
    pushparam t2
    call program_F
    killparam
    killparam
    addi t1 4 t1
    pushparam t1
    aload static_link t1
    pushparam t1
    call program_G
    killparam
    killparam
    killparam
    popparam t1
    stor t1 t0
    load _X t0
    wrii t0
    wrln
    stop
endprogram

subroutine program_F
  parameters
    returnvalue
    static_link
  endparameters

  variables
  endvariables

    load static_link t0
    addi t0 offset(program:_X) t0
    load static_link t1
    addi t1 offset(program:_X) t1
    load t1 t1
    iload 1 t2
    addi t1 t2 t1
    stor t1 t0
    load static_link t1
    addi t1 offset(program:_S) t1
    load static_link t2
    addi t2 offset(program:_X) t2
    load t2 t2
    muli t2 44 t2
    addi t1 t2 t1
    load returnvalue t0
    copy t1 t0 44
    retu
endsubroutine

subroutine program_G
  parameters
    returnvalue
    _A0
    _A1
    static_link
  endparameters

  variables
  endvariables

    load _A0 t0
    iload 0 t1
    muli t1 4 t1
    addi t0 t1 t0
    load t0 t0
    load _A1 t1
    iload 1 t2
    muli t2 4 t2
    addi t1 t2 t1
    load t1 t1
    addi t0 t1 t0
    stor t0 returnvalue
    retu
endsubroutine

Executing code:
8
