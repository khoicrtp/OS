// A simple program uses bubble sort.
#include "syscall.h"

void Bubble_Sort_Ascending(int arr[], int n)
{
  int i, j, temp;
  for (i = 0; i < n - 1; i++)
  {
    for (j = n - 1; j > i; j--)
    {
      if (arr[j - 1] > arr[j])
      {
        temp = arr[j - 1];
        arr[j - 1] = arr[j];
        arr[j] = temp;
      }
    }
  }
}

void Bubble_Sort_Descending(int arr[], int n)
{
  int i, j, temp;
  for (i = 0; i < n - 1; i++)
  {
    for (j = n - 1; j > i; j--)
    {
      if (arr[j - 1] < arr[j])
      {
        temp = arr[j - 1];
        arr[j - 1] = arr[j];
        arr[j] = temp;
      }
    }
  }
}

int main()
{
  int n;
  int i;
  int *arr;
  int choice;
  PrintString("Please input size of the array (<=100): ");
  n = ReadNum();
  arr[n];
  if (n > 100)
  {
    PrintString("Please input number <= 100!!!\n");
    Halt(); //call halt to end program if number <= 100
  }
  else
  {
    for (i = 0; i < n; i++) //get input for array
    {
      PrintString("Input a[");PrintNum(i); PrintString("]: ");
      arr[i] = ReadNum();
    }
    PrintString("Enter 0 if you want to sort descending and 1 if you want to sort ascending: ");
    choice = ReadNum();
    if (choice == 1)
      Bubble_Sort_Ascending(arr, n);

    else if (choice == 0)
      Bubble_Sort_Descending(arr, n);
    else
    {
      PrintString("You're entering a not (0,1) number, so the default will be ascending!!\n");
      Bubble_Sort_Ascending(arr, n);
    }
    for (i = 0; i < n; i++) //Print sorted array
    {
      PrintNum(arr[i]);
      PrintChar(' ');
    }
    
  }
  PrintString("\n");
  Halt();
  /* not reached */
}