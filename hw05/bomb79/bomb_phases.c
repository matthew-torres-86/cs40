extern void explode_bomb();
extern int string_length();
extern int strings_not_equal(char *,char*);

// void phase_5(char *input){
//   if(string_length(input) > 6){
//     explode_bomb();
//   }
//   int curr;
//   char * new_string = "";
//   for(int i = 0; i < 6; i++){
//     curr = input[i] % 16;
//     switch(curr){
//       case 0: new_string[i] = 's';
//       break;
//       case 1: new_string[i] = 'r';
//       break;
//       case 2: new_string[i] = 'v';
//       break;
//       case 3: new_string[i] = 'e';
//       break;
//       case 4: new_string[i] = 'a';
//       break;
//       case 5: new_string[i] = 'w';
//       break;
//       case 6: new_string[i] = 'h';
//       break;
//       case 7: new_string[i] = 'o';
//       break;
//       case 8: new_string[i] = 'b';
//       break;
//       case 9: new_string[i] = 'p';
//       break;
//       case 10: new_string[i] = 'n';
//       break;
//       case 11: new_string[i] = 'v';
//       break;
//       case 12: new_string[i] = 't';
//       break;
//       case 13: new_string[i] = 'f';
//       break;
//       case 14: new_string[i] = 'g';
//       break;
//       case 15: new_string[i] = 'i';
//       break;
//     }
//   }
//     if(strings_not_equal(new_string, "titans")){
//         explode_bomb();
//     }
// }
struct node{
    int data;
    int index;
    node *next;
};


void phase_6(int *values){
    int input[6] = read_six_numbers(stdin);
    node* first = malloc(sizeof(node));
    node *curr = first;
    for(int i = 0; i < 6; i++){
        curr->index = i;
        curr->data = values[i];
        node* next = malloc(sizeof(node));
        curr->next = next;
        curr = curr->next;
    }
    //sort the values from greatest to least
    node *start = first; 
    while(node != NULL)
    {

        while (curr->next !=NULL)
        {
           if(curr->data < curr->next->data)
            {
              node *temp = curr->data;
              curr->data = curr->next->data;
              curr->next->data = temp;
            }
        curr = curr->next;   
        }
        start = start->next; 
    }
    curr=first;
    for(int i = 0; i<6; i++){
        if(curr->index != input[i]){
            explode_bomb();
        }
    }
}