#ifndef BPlusTree_H
#define BPlusTree_H

#include <iostream>
#include <fstream>
#include <string>

template <typename T>
struct Node {
    bool is_leaf;
    std::size_t degree; // maximum number of children
    std::size_t size; // current number of item
    T* item;
    size_t* encode;
    Node<T>** children;
    Node<T>* parent;

public:
    Node(std::size_t _degree) {// Constructor
        this->is_leaf = false;
        this->degree = _degree;
        this->size = 0;

        T* _item = new T[degree-1];
        size_t* _encode = new size_t[degree-1];
        for(int i=0; i<degree-1; i++){
            _item[i] = " ";
            _encode[i] = -1;
        }
        this->item = _item;
        this->encode = _encode;

        Node<T>** _children = new Node<T>*[degree];
        for(int i=0; i<degree; i++){
            _children[i] = nullptr;
        }
        this->children = _children;

        this->parent = nullptr;

    }
};

template <typename T>
class BPlusTree {
    Node<T>* root;
    std::size_t degree;

public:
    BPlusTree(std::size_t _degree) {// Constructor
        this->root = nullptr;
        this->degree = _degree;
    }
    ~BPlusTree() { // Destructor
        clear(this->root);
    }

    Node<T>* getroot(){
        return this->root;
    }

    Node<T>* BPlusTreeSearch(Node<T>* node, T key){
        if(node == nullptr) { // if root is null, return nullptr
            return nullptr;
        }
        else{
            Node<T>* cursor = node; // cursor finding key

            while(!cursor->is_leaf){ // until cusor pointer arrive leaf
                for(int i=0; i<cursor->size; i++){ //in this index node, find what we want key
                    if(key < cursor->item[i]){ //find some range, and let find their child also.
                        cursor = cursor->children[i];
                        break;
                    }
                    if(i == (cursor->size)-1){
                        cursor = cursor->children[i+1];
                        break;
                    }
                }
            }

            //search for the key if it exists in leaf node.
            for(int i=0; i<cursor->size; i++){
                if(cursor->item[i] == key){
                    return cursor;
                }
            }

            return nullptr;
        }
    }
    Node<T>* BPlusTreeRangeSearch(Node<T>* node, T key){
        if(node == nullptr) { // if root is null, return nullptr
            return nullptr;
        }
        else{
            Node<T>* cursor = node; // cursor finding key

            while(!cursor->is_leaf){ // until cusor pointer arrive leaf
                for(int i=0; i<cursor->size; i++){ //in this index node, find what we want key
                    if(key < cursor->item[i]){ //find some range, and let find their child also.
                        cursor = cursor->children[i];
                        break;
                    }
                    if(i == (cursor->size)-1){
                        cursor = cursor->children[i+1];
                        break;
                    }
                }
            }
            return cursor;
        }
    }
    int range_search(T start, T end, T* result_data, int arr_length) {
        int index=0;

        Node<T>* start_node = BPlusTreeRangeSearch(this->root,start);
        Node<T>* cursor = start_node;
        T temp= cursor->item[0];

        while(temp<=end){
            if(cursor == nullptr){
                break;
            }
            for(int i=0; i< cursor->size;i++){
                temp = cursor->item[i];
                if((temp >= start)&&(temp <= end)){
                    result_data[index] = temp;
                    index++;
                }
            }
            cursor = cursor->children[cursor->size];
        }
        return index;
    }
    bool search(T data) {  // Return true if the item exists. Return false if it does not.
        return BPlusTreeSearch(this->root, data) != nullptr;
    }

    int find_index(T* arr, T data, int len){
        int index = 0;
        for(int i=0; i<len; i++){
            if(data == arr[i]){
                index=i;
                break;
            }
            if(data < arr[i]){
                index = i;
                break;
            }
            if(i==len-1){
                index = len;
                break;
            }
        }
        return index;
    }
    std::pair<T*, size_t*> item_insert(T* arr, size_t* arr2, T data, size_t data2, int len){
        int index = 0;
        for(int i=0; i<len; i++){
            if(data < arr[i]){
                index = i;
                break;
            }
            if(i==len-1){
                index = len;
                break;
            }
        }

        for(int i = len; i > index; i--){
            arr[i] = arr[i-1];
            arr2[i] = arr2[i-1];
        }

        arr[index] = data;
        arr2[index] = data2;

        std::pair<T*, size_t*> PAIR; 
        PAIR.first = arr;
        PAIR.second = arr2;
        
        return PAIR;
    }

    Node<T>** child_insert(Node<T>** child_arr, Node<T>*child,int len,int index){
        for(int i= len; i > index; i--){
            child_arr[i] = child_arr[i - 1];
        }
        child_arr[index] = child;
        return child_arr;
    }

    Node<T>* child_item_insert(Node<T>* node, T data, size_t encode_data, Node<T>* child){
        int item_index=0;
        int child_index=0;
        for(int i=0; i< node->size; i++){
            if(data < node->item[i]){
                item_index = i;
                child_index = i+1;
                break;
            }
            if(i==node->size-1){
                item_index = node->size;
                child_index = node->size+1;
                break;
            }
        }
        for(int i = node->size;i > item_index; i--){
            node->item[i] = node->item[i-1];
            node->encode[i] = node->encode[i-1];
        }
        for(int i=node->size+1;i>child_index;i--){
            node->children[i] = node->children[i-1];
        }

        node->item[item_index] = data;
        node->encode[item_index] = encode_data;
        node->children[child_index] = child;

        return node;
    }

    void InsertPar(Node<T>* par,Node<T>* child, T data, size_t encode_data){
        //overflow check
        Node<T>* cursor = par;
        if(cursor->size < this->degree-1){//not overflow, just insert in the correct position
            //insert item, child, and reallocate
            cursor = child_item_insert(cursor,data,encode_data,child);
            cursor->size++;
        }
        else{//overflow
            //make new node
            auto* Newnode = new Node<T>(this->degree);
            Newnode->parent = cursor->parent;

            //copy item
            T* item_copy = new T[cursor->size+1];
            size_t* encode_copy = new size_t[cursor->size+1];
            for(int i=0; i<cursor->size; i++){
                item_copy[i] = cursor->item[i];
                encode_copy[i] = cursor->encode[i];
            }

            std::pair<T*, size_t*> PAIR;
            PAIR= item_insert(item_copy,encode_copy, data, encode_data, cursor->size);
            item_copy = PAIR.first;
            encode_copy = PAIR.second;

            auto** child_copy = new Node<T>*[cursor->size+2];
            for(int i=0; i<cursor->size+1;i++){
                child_copy[i] = cursor->children[i];
            }
            child_copy[cursor->size+1] = nullptr;
            child_copy = child_insert(child_copy,child,cursor->size+1,find_index(item_copy,data,cursor->size+1));

            //split nodes
            cursor->size = (this->degree)/2;
            if((this->degree) % 2 == 0){
                Newnode->size = (this->degree) / 2 -1;
            }
            else{
                Newnode->size = (this->degree) / 2;
            }

            for(int i=0; i<cursor->size;i++){
                cursor->item[i] = item_copy[i];
                cursor->encode[i] = encode_copy[i];
                cursor->children[i] = child_copy[i];
            }
            cursor->children[cursor->size] = child_copy[cursor->size];
            //todo 안지워짐. 뒤에것.

            for(int i=0; i < Newnode->size; i++){
                Newnode->item[i] = item_copy[cursor->size + i +1];
                Newnode->encode[i] = encode_copy[cursor->size + i +1];
                Newnode->children[i] = child_copy[cursor->size+i+1];
                Newnode->children[i]->parent=Newnode;
            }
            Newnode->children[Newnode->size] = child_copy[cursor->size+Newnode->size+1];
            Newnode->children[Newnode->size]->parent=Newnode;

            T paritem = item_copy[this->degree/2];
            size_t parencode = encode_copy[this->degree/2];

            delete[] item_copy;
            delete[] encode_copy;
            delete[] child_copy;

            //parent check
            if(cursor->parent == nullptr){//if there are no parent node(root case)
                auto* Newparent = new Node<T>(this->degree);
                cursor->parent = Newparent;
                Newnode->parent = Newparent;

                Newparent->item[0] = paritem;
                Newparent->encode[0] = parencode;
                Newparent->size++;

                Newparent->children[0] = cursor;
                Newparent->children[1] = Newnode;

                this->root = Newparent;

                //delete Newparent;
            }
            else{//if there already have parent node
                InsertPar(cursor->parent, Newnode, paritem, parencode);
            }
        }
    }

    void insert(T data, size_t encoded_data) {
        if(this->root == nullptr){ //if the tree is empty
            this->root = new Node<T>(this->degree);
            this->root->is_leaf = true;
            this->root->item[0] = data;
            this->root->encode[0] = encoded_data;
            this->root->size = 1; //
        }
        else{ //if the tree has at least one node
            Node<T>* cursor = this->root;

            //move to leaf node
            cursor = BPlusTreeRangeSearch(cursor, data);

            //overflow check
            if(cursor->size < (this->degree-1)){ // not overflow, just insert in the correct position
                //item insert and rearrange
                std::pair<T*, size_t*> PAIR;
                PAIR= item_insert(cursor->item,cursor->encode, data, encoded_data, cursor->size);
                cursor->item = PAIR.first;
                cursor->encode = PAIR.second;
                cursor->size++;
                //edit pointer(next node)
                cursor->children[cursor->size] = cursor->children[cursor->size-1];
                cursor->children[cursor->size-1] = nullptr;
            }
            else{//overflow case
                //make new node
                auto* Newnode = new Node<T>(this->degree);
                Newnode->is_leaf = true;
                Newnode->parent = cursor->parent;

                //copy item
                T* item_copy = new T[cursor->size+1];
                size_t* encode_copy = new size_t[cursor->size+1];
                for(int i=0; i<cursor->size; i++){
                    item_copy[i] = cursor->item[i];
                    encode_copy[i] = cursor->encode[i];
                }

                //insert and rearrange
                
                std::pair<T*, size_t*> PAIR;
                PAIR= item_insert(item_copy,encode_copy, data, encoded_data, cursor->size);
                item_copy = PAIR.first;
                encode_copy = PAIR.second;

                //split nodes
                cursor->size = (this->degree)/2;
                if((this->degree) % 2 == 0){
                    Newnode->size = (this->degree) / 2;
                }
                else{
                    Newnode->size = (this->degree) / 2 + 1;
                }

                for(int i=0; i<cursor->size;i++){
                    cursor->item[i] = item_copy[i];
                    cursor->encode[i] = encode_copy[i];
                }
                for(int i=0; i < Newnode->size; i++){
                    Newnode->item[i] = item_copy[cursor->size + i];
                    Newnode->encode[i] = encode_copy[cursor->size + i];
                }

                cursor->children[cursor->size] = Newnode;
                Newnode->children[Newnode->size] = cursor->children[this->degree-1];
                cursor->children[this->degree-1] = nullptr;

                delete[] item_copy;
                delete[] encode_copy;

                //parent check
                T paritem = Newnode->item[0];
                size_t parencode = Newnode->encode[0];

                if(cursor->parent == nullptr){//if there are no parent node(root case)
                    auto* Newparent = new Node<T>(this->degree);
                    cursor->parent = Newparent;
                    Newnode->parent = Newparent;

                    Newparent->item[0] = paritem;
                    Newparent->encode[0] = parencode;
                    Newparent->size++;

                    Newparent->children[0] = cursor;
                    Newparent->children[1] = Newnode;

                    this->root = Newparent;
                }
                else{//if there already have parent node
                    InsertPar(cursor->parent, Newnode, paritem, parencode);
                }
            }
        }
    }

    void clear(Node<T>* cursor){
        if(cursor != nullptr){
            if(!cursor->is_leaf){
                for(int i=0; i <= cursor->size; i++){
                    clear(cursor->children[i]);
                }
            }
            delete[] cursor->item;
            delete[] cursor->children;
            delete cursor;
        }
    }

    void bpt_print(){
        print(this->root);
    }
    void print(Node<T>* cursor) {
        // You must NOT edit this function.
        if (cursor != NULL) {
            if (cursor->is_leaf){
                for (int i = 0; i < cursor->size; ++i) {
                    std::cout << cursor->item[i]  << " " << cursor->encode[i]<< " ";
                }
            }
            std::cout << "\n";

            if (!cursor->is_leaf) {
                for (int i = 0; i < cursor->size + 1; ++i) {
                    print(cursor->children[i]);
                }
            }
        }
    }

    void bpt_write(char* outfile){
        std::ofstream os(outfile);
        write(this->root, os);
        os << "\n";
        os.close();
    }
    void write(Node<T>* cursor, std::ostream &os) {
        // You must NOT edit this function.
        if (cursor != NULL) {
            if (cursor->is_leaf){
                for (int i = 0; i < cursor->size; ++i) {
                    os << cursor->item[i]  << " " << cursor->encode[i]<< " ";
                }
            }
            os << "\n";

            if (!cursor->is_leaf) {
                for (int i = 0; i < cursor->size + 1; ++i) {
                    write(cursor->children[i], os);
                }
            }
        }
    }
};

#endif