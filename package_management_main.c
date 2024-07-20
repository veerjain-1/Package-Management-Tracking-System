#include <string.h>
#include <stdio.h>

#include "hw5.h"

void print_package(package_t pack);
package_t read_package_from_file(char *);
void print_return_value(int, char *);

/* This function is used to run the different functions implemented in file
 * hw5.c. User input is used to determine which function to run and what input
 * is passed to it. Upon completion, the function returns zero.
 */

int main() {
  char database_file[100] = "";
  FILE * database_fp = 0;
  char pack_file[100] = "";
  int return_code = 0;
  int pack_id = 0;
  float ret = 0.0;
  int oid = 0;
  char dest[MAX_NAME_LEN] = {0};

  package_t pack = BAD_PACKAGE;

  printf("\nEnter the name of an input file to use: ");
  fgets(database_file, sizeof(database_file), stdin);
  *(strchr(database_file, '\n')) = '\0';

  database_fp = fopen(database_file, "rb+");
  if (database_fp == NULL) {
    printf("Unable to open %s for input.\n", database_file);
    return 1;
  }

  while (1) {

    printf("\nOPTIONS:\n"
           "0) Quit\n"
           "1) read_package()\n"
           "2) write_package()\n"
           "3) recv_package()\n"
           "4) send_package()\n"
           "5) send_to_route()\n"
           "6) find_package_by_id()\n"
           "7) find_shipping_cost()\n"
           "8) raise_priority()\n"
           "9) combine_package()\n"
           "Select a function: ");
    int choice = 0;
    int status = fscanf(stdin, "%d", &choice);
    /* To prevent infinite loop incase of non integer input. */
    while (getchar() != '\n'){

    }
    if (status <= 0) {
      printf("\nInvalid input! Try again...\n");
      continue;
    }
    if (choice == 0) {
      printf("\nGoodbye!\n\n");
      break;
    }

    int record_num = 0;
    switch (choice) {
      case 0:
        printf("\nGoodbye!\n\n");
        break;
      case 1:
        printf("\nEnter package record number: ");
        fscanf(stdin, "%d", &record_num);
        pack = read_package(database_fp, record_num);
        print_package(pack);
        break;
      case 2:
        printf("\nEnter package record number: ");
        fscanf(stdin, "%d", &record_num);
        printf("Enter a file containing an package representation: ");
        fscanf(stdin, "\n%s", pack_file);
        pack = read_package_from_file(pack_file);
        printf("Read the following information:\n");
        print_package(pack);
        return_code = write_package(database_fp, pack, record_num);
        print_return_value(return_code, "write_package");
        break;
      case 3:
        printf("\nEnter a file containing an package representation: ");
        fscanf(stdin, "\n%s", pack_file);
        pack = read_package_from_file(pack_file);
        printf("Read the following information:\n");
        print_package(pack);
        return_code = recv_package(database_fp, pack);
        print_return_value(return_code, "recv_package");
        break;
      case 4:
        int pid; 
        printf("\nEnter package id: ");
        fscanf(stdin, "%d", &pid);
        return_code = send_package(database_fp, pid);
        print_return_value(return_code, "send_package");
        break;
      case 5:
        printf("\nEnter a route: ");
        char truck[MAX_ROUTE_LEN];
        fscanf(stdin, "%10[^\n]", truck);
        ret = send_to_route(database_fp, truck);
        printf("Value returned by %s was %f", "send_to_route", ret);
        break;
      case 6:
        printf("\nEnter a package ID: ");
        fscanf(stdin, "%d", &pack_id);
        pack = find_package_by_id(database_fp, pack_id);
        printf("package returned by %s:", "find_package_by_id");
        print_package(pack);
        break;
      case 7:
        printf("\nEnter an order ID: ");
        fscanf(stdin, "%d", &oid);
        ret = find_shipping_cost(database_fp, oid);
        printf("Value returned by %s was %f", "find_shipping_cost", ret);
        break;
      case 8:
        printf("\nEnter a package ID: ");
        fscanf(stdin, "%d", &pack_id);
        ret = raise_priority(database_fp, pack_id);
        printf("Value returned by %s was %f", "raise_priority", ret);
        break;
      case 9:
        printf("\nEnter an order ID: ");
        fscanf(stdin, "%d", &oid);
        printf("Enter a destination: ");
        fscanf(stdin, "%19[^\n]", dest);
        pack = combine_package(database_fp, oid, dest);
        printf("package returned by %s:", "combine_package");
        print_package(pack);
        break;
      default:
        printf("\nInvalid selection.\n");
        break;
    }
  }

  return 0;
} /* main() */


/*
 * Function to read in information about an package
 */

package_t read_package_from_file(char *file_name) {
  FILE *fp = fopen(file_name, "r");
  package_t pack = BAD_PACKAGE;
  if (fp == NULL){
    return pack;
  }
  if (fscanf(fp, "%d\n", &pack.id) != 1){

    return pack;
  }
   
  if (fscanf(fp, "%d\n", &pack.order_id) != 1){


    return pack;
  }
  
  if (fscanf(fp, "%19s\n", pack.dest) != 1){
    return pack;
  }
  if (fscanf(fp, "%d\n", ((int *)&pack.priority)) != 1){
    return pack;
  }
  if (fscanf(fp, "%f\n", &pack.value) != 1){
    return pack;
  }
   if (fscanf(fp, "%f\n", &pack.weight) != 1){
    return pack;
  }
  for (int i = 0; i < N_ROUTES; i++){
    for (int j = 0; j < MAX_ROUTE_LEN; j++){
      if (fscanf(fp, "%c", &pack.routes[i][j]) != 1){
        return pack;
      }
    }
    
    if (fscanf(fp, "\n") != 0){
      return pack;
    }
  }
    printf("ok");

  return pack;
} /* read_package_from_file() */


/*
 * Function to print package information
 */

void print_package(package_t pack) {
  printf("\nId: %d\nOrder id: %d\nDestination: %.19s\n", pack.id,
         pack.order_id, pack.dest);
  switch (pack.priority){
    case REGULAR:
      printf("Priority: Regular\n");
      break;
    case URGENT:
      printf("Priority: URGENT\n");
      break;
    case PRIME:
      printf("Priority: PRIME\n");
      break;
    case PRIME_URGENT:
      printf("Priority: PRIME URGENT\n");
      break;
    default:
      printf("Priority: INVALID\n");
      break;
  }
  printf("Value: %.2f\n", pack.value);
  printf("Weight: %.2f\n", pack.weight);
  printf("Routes:\n");
  for (int i = 0; i < N_ROUTES; i++){
    for (int j = 0; j < MAX_ROUTE_LEN; j++){
      printf("%c", pack.routes[i][j]);
    }
    printf("\n");
  }
} /* print_package() */


/*
 * Function to print appropriate error codes.
 */

void print_return_value(int val, char *function) {
  printf("Value returned by %s(): %d ", function, val);
  switch(val) {
    case OK:
      printf("(OK)\n");
      break;
    case NO_PACKAGE:
      printf("(NO_PACKAGE)\n");
      break;
    case NO_SPACE:
      printf("(NO_SPACE)\n");
      break;
    case WRITE_ERR:
      printf("(WRITE_ERR)\n");
      break;
    case MALFORMED_PACKAGE:
      printf("(MALFORMED_PACKAGE)\n");
      break;
    default:
      printf("\n");
      break;
  }
} /* print_return_value() */
