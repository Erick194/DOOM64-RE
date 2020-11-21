

int mystrlen(char *string) // 80001CE8
{
  int rc = 0;
  if (string) while (*(string++)) rc++;
  else rc = -1;
  return rc;
}

int D_vsprintf(char *string, const char *format, int *argptr) // 80001D24
{
  int len, i, div, uselong;
  int fieldsize;
  unsigned long num;
  long snum;
  char padchar;
  char *str;
  char *origstring = string;

  while (*format)
  {
    if (*format != '%') *(string++) = *(format++);
    else
    {
      format++;

      /* set field pad character to 0 if necessary */
      if (*format == '0')
      {
        padchar = '0';
        format++;
      }
      else padchar = ' ';

      /* get the fieldwidth if any */
      fieldsize = 0;
      while (*format >= '0' && *format <= '9')
	fieldsize = fieldsize * 10 + *(format++) - '0';

      /* get rid of 'l' if present */
      if (*format == 'l')
      {
        uselong = 1;
        format++;
      } else uselong = 0;

      div = 10;
      if (*format == 'c')
      {
	*(string++) = *argptr++;
	format++;
      }
      else if (*format == 's')
      {
	str = (char *)*argptr++;
	len = mystrlen(str);
	while (fieldsize-- > len) *(string++) = padchar; /* do field pad */
	while (*str) *(string++) = *(str++); /* copy string */
	format++;
      }
      else
      {
        if (*format == 'o') /* octal */
        {
          div = 8;
          if (uselong)
	    num = *argptr++;
	  else
	    num = *argptr++;
/*	  printf("o=0%o\n", num); */
        }
        else if (*format == 'x' || *format == 'X')  /* hex */
        {
          div = 16;
          if (uselong)
	    num = *argptr++;
	  else
	    num = *argptr++;
/*	  printf("x=%x\n", num); */
	}
        else if (*format == 'i' || *format == 'd' || *format == 'u') /* decimal */
        {
          div = 10;
          if (uselong)
	    snum = *argptr++;
	  else
	    snum = *argptr++;
	  if (snum < 0 && *format != 'u') /* handle negative %i or %d */
	  {
	    *(string++) = '-';
	    num = -snum;
	    if (fieldsize) fieldsize--;
	  } else num = snum;
	}
	else return -1; /* unrecognized format specifier */

	/* print any decimal or hex integer */
	len = 0;
	while (num || fieldsize || !len)
	{
	  for (i=len ; i ; i--) string[i] = string[i-1]; /* shift right */
	  if (len && fieldsize && !num) *string = padchar; /* pad out */
	  else
	  {
	    /* put in a hex or decimal digit */
	    *string = num % div;
	    *string += *string > 9 ? 'A'-10 : '0';
/*	    printf("d = %c\n", *string); */
	    num /= div;
	  }
	  len++;
	  if (fieldsize) fieldsize--;
	}
	string += len;
	format++;
      }
    }
  }
  *string = 0;

  return origstring - string;
}
