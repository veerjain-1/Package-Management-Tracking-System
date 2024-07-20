#include "package_management.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function Definitions */

int match_route(char *, char*);
char* get_route(char *);
int get_priority_integer(enum priority_t);

package_t read_package(FILE *file, int n) {
    assert((file != NULL) && (n >= 0));
    package_t package = {};
    fseek(file, n * sizeof(package_t), SEEK_SET);
    if ((fread(&package, sizeof(package_t), 1, file)) != 1) {
        return BAD_PACKAGE;
    }
    return package;
}

int write_package(FILE *file, package_t package, int n) {
    assert((file != NULL) && (n >= 0));
    if ((package.id <= 0) || (package.order_id <= 0) || (package.value <= 0.0) ||
        (package.weight <= 0.0) ||
        ((package.priority != REGULAR) && (package.priority != URGENT) &&
         (package.priority != PRIME) && (package.priority != PRIME_URGENT))) {
        return MALFORMED_PACKAGE;
    }
    int size_of_package = sizeof(package_t);
    long offset = n * size_of_package;
    fseek(file, 0L, SEEK_END);
    long file_size = ftell(file);
    if (offset > file_size) {
        return WRITE_ERR;
    }
    int seek_result = fseek(file, offset, SEEK_SET);
    if (seek_result == 0) {
        size_t size_of_write = fwrite(&package, sizeof(package_t), 1, file);
        if (size_of_write) {
            return OK;
        }
    }
    return WRITE_ERR;
}

int recv_package(FILE *file, package_t package) {
    assert(file != NULL);
    if ((package.id <= 0) || (package.order_id <= 0) || (package.value <= 0.0) ||
        (package.weight <= 0.0) || ((package.priority != REGULAR) &&
                                    (package.priority != URGENT) && (package.priority != PRIME) &&
                                    (package.priority != PRIME_URGENT))) {
        return MALFORMED_PACKAGE;
    }
    package_t temp_package = {};
    fseek(file, 0, SEEK_SET);
    int position = 0;
    while (fread(&temp_package, sizeof(package_t), 1, file) == 1) {
        if (temp_package.id == FREE_ID) {
            fseek(file, position * sizeof(package_t), SEEK_SET);
            if (fwrite(&package, sizeof(package_t), 1, file) == 1) {
                fflush(file);
                return position;
            }
            else {
                return WRITE_ERR;
            }
        }
        position++;
    }
    return NO_SPACE;
}

int send_package(FILE *file, int id){
    assert((file != NULL) && (id >= 0));
    package_t temp_package = {};
    int placement = 0;
    fseek(file, 0, SEEK_SET);
    while (fread(&temp_package, sizeof(package_t), 1, file) == 1) {
        if (temp_package.id == id) {
            temp_package.id = FREE_ID;
            fseek(file, placement * sizeof(package_t), SEEK_SET);
            if (fwrite(&temp_package, sizeof(package_t), 1, file) != 1) {
                return placement;
            }
            return placement;
        }
        placement++;
    }
    return NO_PACKAGE;
}

int match_route(char* route, char* pkg_route) {
    int route_pos = 0;
    for (int i = 0; (pkg_route[i] != '\0') && (pkg_route[i] != '.'); ++i) {
        char ch = pkg_route[i];
        int found = 0;
        while (route[route_pos] != '\0') {
            if (route[route_pos] == ch) {
                found = 1;
                break;
            }
            ++route_pos;
        }
        if (!found) {
            return -1;
        }
        ++route_pos;
    }
    return 1;
}

int letter_in_route(char* route, int route_start_pos, char pkg_route_letter){
    for (int i = route_start_pos; i < MAX_ROUTE_LEN; i++) {
        if (route[i] == pkg_route_letter) {
            return i;
        }
    }
    return -1;
}

float send_to_route(FILE *file_in, char *route) {
    assert((file_in != NULL) && (route != NULL));
    fseek(file_in, 0, SEEK_SET);
    int check = 0;
    float sum = 0.0;
    package_t package = BAD_PACKAGE;
    while (fread(&package, sizeof (package_t), 1, file_in) == 1) {
        if (package.id == FREE_ID) {
            continue;
        }
        for (int i = 0; i < N_ROUTES; i++) {
            int present = 1;
            int endex = 0;
            for (int route_dex = 0; (route_dex < MAX_ROUTE_LEN) &&
                                    (package.routes[i][route_dex] != '.'); route_dex++) {
                int idx_found = letter_in_route(route, endex,
                                                package.routes[i][route_dex]);
                if (idx_found == -1) {
                    present = 0;
                    route_dex = MAX_ROUTE_LEN;
                }
                else {
                    endex = idx_found;
                }
            }
            if (present == 1) {
                sum += package.weight;
                package.id = FREE_ID;
                fseek(file_in, check * sizeof (package_t), SEEK_SET);
                fwrite(&package, sizeof (package_t), 1, file_in);
                break;
            }
        }
        check += 1;
    }
    if (sum == 0.0) {
        return NO_PACKAGE;
    }
    else {
        return sum;
    }
}

package_t find_package_by_id(FILE *file, int id){
    assert((file != NULL) && (id >= 0));
    fseek(file, 0, SEEK_SET);
    package_t temp_package = {};
    while (fread(&temp_package, sizeof(package_t), 1, file) == 1) {
        if (temp_package.id == id) {
            return temp_package;
        }
    }
    return BAD_PACKAGE;
}

char* get_route(char *a) {
    char *b = malloc(MAX_ROUTE_LEN);
    int i = 0;
    for (; i < MAX_ROUTE_LEN; i++) {
        if (a[i] == '.') {
            break;
        }
        b[i] = a[i];
    }
    b[i] = '\0';
    return b;
}

int calculate_max_route_length(package_t temp_package) {
    int max_length = 0;
    for (int i = 0; i < N_ROUTES; i++) {
        int route_length = strlen(get_route(temp_package.routes[i]));
        if (route_length > max_length) {
            max_length = route_length;
        }
    }
    return max_length;
}

float calculate_cost(package_t temp_package, int max_length) {
    float cost = temp_package.weight * 2.5 * max_length;
    cost += (temp_package.weight > 100.0) ? 80.0 : 0.0;
    cost += (temp_package.value > 200.0) ? 50.0 : 0.0;
    switch (temp_package.priority) {
        case URGENT:
        case PRIME_URGENT:
            cost += 120.0;
            break;
        default:
            break;
    }
    return cost;
}

float find_shipping_cost(FILE *file, int order_id) {
    assert((file != NULL) && (order_id >= 0));
    package_t temp_package = {};
    float total_cost = 0.0;
    int package_found = 0;
    fseek(file, 0, SEEK_SET);
    while (fread(&temp_package, sizeof(package_t), 1, file) == 1) {
        if (temp_package.order_id == order_id) {
            int max_length = calculate_max_route_length(temp_package);
            float cost = calculate_cost(temp_package, max_length);
            total_cost += cost;
            package_found = 1;
        }
    }
    if (package_found == 0) {
        return NO_PACKAGE;
    }
    return total_cost;
}

float raise_priority(FILE *file, int id) {
    assert((file != NULL) && (id >= 0));
    fseek(file, 0, SEEK_SET);
    package_t package = {};
    while (fread(&package, sizeof(package_t), 1, file) == 1) {
        if (package.id == id) {
            if (package.priority == REGULAR) {
                package.priority = URGENT;
            }
            else if (package.priority == PRIME) {
                package.priority = PRIME_URGENT;
            }
            fseek(file, -sizeof(package_t), SEEK_CUR);
            fwrite(&package, sizeof(package_t), 1, file);
            return package.value;
        }
    }
    return NO_PACKAGE;
}

int get_priority_integer(enum priority_t priority) {
    switch (priority) {
        case REGULAR: return 0;
        case URGENT: return 1;
        case PRIME: return 2;
        case PRIME_URGENT: return 3;
    }
    return -1;
}

package_t combine_package(FILE *file, int order_id, char* destination) {
    assert((file != NULL) && (order_id >= 0) && (destination != NULL));
    fseek(file, 0, SEEK_SET);
    package_t temp_package = {};
    package_t combined_package = BAD_PACKAGE;
    while (fread(&temp_package, sizeof(package_t), 1, file) == 1) {
        if ((temp_package.order_id == order_id) &&
            (strcmp(temp_package.dest, destination) == 0) &&
            (temp_package.id != FREE_ID)) {
            if (combined_package.id == BAD_PACKAGE.id) {
                combined_package = temp_package;
            }
            else {
                combined_package.value += temp_package.value;
                combined_package.weight += temp_package.weight +
                                           (0.15 * fmax(combined_package.weight, temp_package.weight));
                for (int i = 0; i < N_ROUTES; i++) {
                    if (strlen(get_route(temp_package.routes[i])) <
                        strlen(get_route(combined_package.routes[i]))) {
                        memcpy(combined_package.routes[i], temp_package.routes[i], 10);
                    }
                }
                if (get_priority_integer(temp_package.priority) >
                    get_priority_integer(combined_package.priority)) {
                    combined_package.priority = temp_package.priority;
                }
            }
        }
    }
    return combined_package;
}
