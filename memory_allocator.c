#include<stdio.h>
#include<stdlib.h>
#include<string.h>

unsigned char *a; /// the array used for saving all the memory in the arena
int start, N, poz;
int util_zero;

void Initialize(int n)
{
    start = 0;
    N = n;
    a = calloc(n, sizeof(unsigned char));
    if (a==NULL) exit(0);
}

void Finalize()
{
    start = 0;
    util_zero = 0;
    free(a);
}

void Dump()
{
    int i;
    for (i=0; i<N; i++)
    {
        if (i%16 == 0)
        {
            printf("%08X\t", i);
            printf("%02X ", a[i]);
        }
        else if (i%8 == 0)
        {
            printf(" ");
            printf("%02X ", a[i]);
        }
        else
            printf("%02X ", a[i]);

        if (i%16 == 15)
            printf("\n");
    }
}

void Completeaza(int i, int numar)
{
    unsigned char val;
    int nr;
    /// aa[i+0,3] will store the value of the number
    nr = numar;
    val = (nr%256) & 255;
    a[i] = val;
    nr = nr/256;
    val = (nr%256) & 255;
    a[i+1] = val;
    nr = nr/256;
    val = (nr%256) & 255;
    a[i+2] = val;
    nr = nr/256;
    val = (nr%256) & 255;
    a[i+3] = val;
}

void Free(int index)
{
    int cod1, cod2, cod3, i, st, dr;
    cod1 = a[index-12] + a[index-11]*256 + a[index-10]*256*256 + a[index-9]*256*256*256;
    cod2 = a[index-8] +  a[index-7]*256  + a[index-6]*256*256 +  a[index-5]*256*256*256;
    cod3 = a[index-4] +  a[index-3]*256  + a[index-2]*256*256 +  a[index-1]*256*256*256;

    if (cod1 == 0 && cod2 == 0)
    {
        for (i=index-4; i<index+cod3; i++)
            a[i] = 0;
        start = 0;
    }
    else if (cod2 == 0)
    {
        st = index-12;
        dr = index+cod3;
        Completeaza(cod1+4,index-12);
        start = cod1;
        for (i=st; i<dr; i++)
            a[i] = 0;
        /// deleted the leftest memory block
    }
    else if (cod1 == 0)
    {
        st = index-12;
        dr = index+cod3;
        Completeaza(cod2,0);
        for (i=st; i<dr; i++)
            a[i] = 0;
        /// deleted the rightest memory block
    }
    else
    {
         st = index-12;  dr = index+cod3;
         Completeaza(cod2, cod1);
         Completeaza(cod1+4, cod2);

         for (i=st; i<dr; i++)
            a[i] = 0;
        /// deleted a random memory block
    }
}

void Alloc(int size)
{
    int nr, liber=0, next=0, ok=0;
    int cod1, cod3;
    cod1 = cod3 = 0;
    unsigned char val;

    if (start == 0 && util_zero == 0)
    {
        if (size + 12 <= N)
        {
            /// modify only cod3 = size
            util_zero = 1;
            Completeaza(8, size);
            next = 12;
            ok = 1;
        }
        else
            ok = 0;
    }
    else
    {
        if (start == 0 && util_zero == 1)
        {
            poz = start;
            while ( !(a[poz]==0 && a[poz+1]==0 && a[poz+2]==0 && a[poz+3]==0) && ok==0)
            {
                cod1 = a[poz] + a[poz+1]*256 + a[poz+2]*256*256 + a[poz+3]*256*256*256;
                cod3 = a[poz+8] + a[poz+9]*256 + a[poz+10]*256*256 + a[poz+11]*256*256*256;
                liber = cod1 - poz - cod3 - 12;

                if (size + 12 <= liber)
                {
                    ok = 1;
                    next = poz+cod3+24;
                    /// cod1_new
                    Completeaza(poz + cod3 + 12, cod1);
                    /// cod2_new
                    Completeaza(poz + cod3 + 16, poz);
                    /// cod3_new
                    Completeaza(poz + cod3 + 20, size);
                    /// cod2_right
                    Completeaza(cod1 + 4, poz + cod3 + 24);
                    /// cod1_left
                    Completeaza(poz, poz + 12 + cod3);
                }
                else poz = a[poz];
            }

            if (ok==0)   /// am ajuns la ultimul bloc
            {
                cod3 = a[poz+8] + a[poz+9]*256 + a[poz+10]*256*256 + a[poz+11]*256*256*256;
                if (size + 12 <= N - poz - cod3 - 12)
                {
                    ok=1;
                    next=poz + cod3 + 24;
                    /// cod2_new
                    Completeaza(poz + cod3 + 16, poz);
                    /// cod3_new
                    Completeaza(poz + cod3 + 20, size);
                    /// cod1_left
                    Completeaza(poz, poz + 12 + cod3);
                }
                else
                    ok = 0, next = 0;
            }
        }

        else if (start != 0)
        {
            /// incercam sa pun inainte de start
            if (size + 12 <= start)
            {
                ok = 1;
                next = 12;
                /// cod1_new
                Completeaza(0, start);
                /// cod3_new
                Completeaza(8, size);
                /// cod2_right
                Completeaza(start+4, 0);
                start = 0;
            }
            else
            {
                poz = start;
                /// porneste la drum
                while ( !(a[poz]==0 && a[poz+1]==0 && a[poz+2]==0 && a[poz+3]==0) && ok==0)
                {
                    cod1 = a[poz] + a[poz+1]*256 + a[poz+2]*256*256 + a[poz+3]*256*256*256;
                    cod3 = a[poz+8] + a[poz+9]*256 + a[poz+10]*256*256 + a[poz+11]*256*256*256;
                    liber = cod1 - poz - cod3 - 12;

                    if (size + 12 <= liber)
                    {
                        ok = 1;
                        next = poz+12+cod3+12;
                        /// cod1_new
                        Completeaza(poz + cod3 + 12, cod1);
                        /// cod2_new
                        Completeaza(poz + cod3 + 16, poz);
                        /// cod3_new
                        Completeaza(poz + cod3 + 20, size);
                        /// cod2_right
                        Completeaza(cod1 + 4, poz + cod3 + 12);
                        /// cod1_left
                        Completeaza(poz, poz + 12 + cod3);
                    }
                }

                if (ok==0)   /// am ajuns la ultimul bloc
                {
                    cod3 = a[poz+8] + a[poz+9]*256 + a[poz+10]*256*256 + a[poz+11]*256*256*256;

                    if (size + 12 <= N - poz - cod3 - 12)
                    {
                        ok=0;
                        next=0;
                        /// cod2_new
                        Completeaza(poz + cod3 + 16, poz);
                        /// cod3_new
                        Completeaza(poz + cod3 + 20, size);
                        /// cod1_left
                        Completeaza(poz, poz + 12 + cod3);
                    }
                    else
                        ok = 0, next = 0;
                }
            }
        }
    }
    if (ok == 1) printf("%d\n", next);
    else printf("0\n");
}

void Fill(int index, int size, int value)
{
   int ok, i, cod1, cod2, cod3;
    ok = 0;

    while (ok == 0)
    {
        cod1 = a[index-12] + a[index-11]*256 + a[index-10]*256*256 + a[index-9]*256*256*256;
        cod2 = a[index-8] +  a[index-7]*256  + a[index-6]*256*256 +  a[index-5]*256*256*256;
        cod3 = a[index-4] +  a[index-3]*256  + a[index-2]*256*256 +  a[index-1]*256*256*256;

        if (size >= cod3)
        {
        size = size - cod3;
        for (i=index; i<index+cod3; i++)
            a[i] = value;
        }
        else
    {
        for (i=index; i<index+size; i++)
            a[i] = value;
        size = 0;
    }

    if (size == 0)
        ok = 1;
    else
    {
        if (cod1 !=0)
        index = cod1 + 12;
        else
        ok = 1;
   }
 }
}

void Read()
{
   char line[200], s;
   int len=0, ok=0;
   int inv, inv1, inv2, inv3;

   while (s = getchar())
   {
      /// input reading cases

      if (s == 'I')
        {
            s = getchar(), s = getchar();
            s = getchar(), s = getchar();
            s = getchar(), s = getchar();
            s = getchar(), s = getchar();
            s = getchar(), s = getchar(); /// for moving on to the next char
            s = getchar();
            ok = 0; inv = 0;

            while (ok == 0)
            {
              inv = inv*10 + (s-'0');
              s = getchar();
              if (s < '0' || s > '9')
                  ok = 1;
            }

             Initialize(inv);  /// 10 chars - ENTER included
        }

      else if (s == 'D')
        {
            Dump(); /// 4 chars - ENTER included
            s = getchar(), s = getchar();
            s = getchar(), s = getchar();
        }

      else if (s == 'A')
      {
          /// ALLOC number
          s = getchar(), s = getchar();
          s = getchar(), s = getchar();
          s = getchar(), s = getchar();

          ok = 0; inv = 0;

          while (ok == 0)
          {
             inv = inv*10 + (s-'0');
             s = getchar();
             if (s < '0' || s > '9')
                 ok = 1;
          }

          Alloc(inv);
      }

      else if (s == 'F')
      {
          s = getchar();

          if (s == 'R')
            /// FREE index
           {
                 inv = 0;
                 s = getchar(), s = getchar();
                 s = getchar(), s = getchar();

                 ok = 0; inv = 0;

                 while (ok == 0)
                  {
                     inv = inv*10 + (s-'0');
                     s = getchar();
                     if (s < '0' || s > '9')
                       ok = 1;
                  }

                 Free(inv);
           }

          else if (s == 'I')
          {
              s = getchar();
              if (s == 'N')
              {
                 Finalize();

                 s = getchar(), s = getchar();
                 s = getchar(), s = getchar();
                 s = getchar(), s = getchar();
              }
              else
              {
                  /// FILL index size value
                  s = getchar(), s = getchar();
                  inv1 = inv2 = inv3 = 0;
                  ok = 0; s = getchar();

                        while (ok == 0)
                   {
                        inv1 = inv1*10 + (s-'0');
                        s = getchar();
                        if (s < '0' || s > '9')
                        ok = 1;
                   }
                   ok = 0; s = getchar();

                        while (ok == 0)
                   {
                        inv2 = inv2*10 + (s-'0');
                        s = getchar();
                        if (s < '0' || s > '9')
                        ok = 1;
                   }

                  ok = 0; s = getchar();

                        while (ok == 0)
                   {
                        inv3 = inv3*10 + (s-'0');
                        s = getchar();
                        if (s < '0' || s > '9')
                        ok = 1;
                   }

                  Fill(inv1, inv2, inv3);
              }
          }
      }
   }
}


int main ()
{
    Read();
    return 0;
}
