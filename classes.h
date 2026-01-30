/*** This is just a Skeleton/Starter Code for the External Storage Assignment. This is by no means absolute, in terms of assignment approach/ used functions, etc. ***/
/*** You may modify any part of the code, as long as you stick to the assignments requirements we do not have any issue ***/

// Include necessary standard library headers
#include <string>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <bitset>
using namespace std; // Include the standard namespace

class Record {
public:
    int id, manager_id; // Employee ID and their manager's ID
    std::string bio, name; // Fixed length string to store employee name and biography

    Record(vector<std::string> &fields) {
        id = stoi(fields[0]);
        name = fields[1];
        bio = fields[2];
        manager_id = stoi(fields[3]);
    }

    //You may use this for debugging / showing the record to standard output. 
    void print() {
        cout << "\tID: " << id << "\n";
        cout << "\tNAME: " << name << "\n";
        cout << "\tBIO: " << bio << "\n";
        cout << "\tMANAGER_ID: " << manager_id << "\n";
    }

    // Function to get the size of the record
    int get_size() {
        // sizeof(int) is for name/bio size() in serialize function
        return sizeof(id) + sizeof(manager_id) + sizeof(int) + name.size() + sizeof(int) + bio.size(); 
    }
    
    // Take a look at Figure 9.9 and read the Section 9.7.2 [Record Organization for Variable Length Records]
    // TO_DO: Consider using a delimiter in the serialize function to separate these items for easier parsing.
    string serialize() const {
        char dollar = '$';
        string my_string = to_string(id)+ dollar + name + dollar + bio + dollar + to_string(manager_id);
        
        return my_string;
    }
};

class page{ // Take a look at Figure 9.7 and read Section 9.6.2 [Page organization for variable length records] 
public:
    vector <Record> records; // Data Area: Stores records. 
    vector <pair <int, int>> slot_directory; // This slot directory contains the starting position (offset), and size of the record. 
                                        
    int cur_size = 0; // holds the current size of the page

    // Function to insert a record into the page
    bool insert_record_into_page(Record r) {
        int record_size = r.get_size();
        int slot_size = sizeof(int64_t) * 2;

        if (cur_size + record_size + slot_size + 3*sizeof('$') + 1> 4096) { //Check if page size limit exceeded, considering slot directory size
            return false; // Cannot insert the record into this page
        }else {
            records.push_back(r); // Record stored in current page
            
            // TO_DO: update slot directory information
            if(cur_size == 0){
                slot_directory.push_back(make_pair(0, record_size + 3));
            }else{
                //OG: slot_directory.push_back(make_pair(cur_size - (sizeof(int64_t) * 2), record_size + 3));
                slot_directory.push_back({cur_size, record_size});
                cur_size += record_size;

            }
            cur_size += r.get_size(); // Updating page size
            cur_size += 3*sizeof('$');
            cur_size += sizeof(int64_t) * 2;
        }
        return true;
    }




    // Function to write the page to a binary file, i.e., EmployeeRelation.dat file
    void write_into_data_file(ostream& out) const { 
        
        char page_data[4096] = {0}; // Write the page contents (records and slot directory) into this char array so that the page can be written to the data file in one go.

        int offset = 0; // Used as an iterator to indicate where the next item should be stored. Section 9.6.2 contains information that will help you with the implementation.

        for (const auto& record : records) { // Writing the records into the page_data
            string serialized = record.serialize();
            memcpy(page_data + offset, serialized.c_str(), serialized.size());
            offset += serialized.size();
        }
        
        // TO_DO: Put a delimiter here to indicate slot directory starts from here 
        char hash_delimiter = '#';
        memcpy(page_data + offset, &(hash_delimiter), sizeof(char));
        offset += sizeof(hash_delimiter);

        for(const auto& slots : slot_directory) { 
            // TO_DO: Write the slot-directory information into page_data. You'll use slot-directory to retrieve record(s).
            // offset - int of 4 bytes

            int64_t first = static_cast<int64_t>(slots.first);
            int64_t second = static_cast<int64_t>(slots.second);

            memcpy(page_data + offset, &first, sizeof(int64_t));
            offset += sizeof(int64_t);
        
            // length - int of 4 bytes
            memcpy(page_data + offset, &second, sizeof(int64_t));
            offset += sizeof(int64_t);
            
            
        }
        out.write(page_data, sizeof(page_data)); // Write the page_data to the EmployeeRelation.dat file 
    }

    // Read a page from a binary input stream, i.e., EmployeeRelation.dat file to populate a page object
    bool read_from_data_file(istream& in) {
        
        char page_data[4096] = {0}; // Character array used to read 4 KB from the data file to your main memory. 
        in.read(page_data, 4096); // Read a page of 4 KB from the data file 

        // Clear existing data
        records.clear();
        slot_directory.clear();

        streamsize bytes_read = in.gcount(); // used to check if 4KB was actually read from the data file
        // cout << "found " <<bytes_read << "bytes" << endl;
        if (bytes_read == 4096) {
            
            // TO_DO: You may process page_data (4 KB page) and put the information to the records and slot_directory (main memory).
            // read page_data backward from footer until we find '#'
            size_t delimeter_position = 4097;

            for (int i = 0 ; i < bytes_read; i++) {
                //find the record delimeter. - #
                if (page_data[i] == '#') {
                    delimeter_position = i;
                    break;
                }
            }

            // cout << "# found at " << delimeter_position << endl;
            // cout << "char:" << page_data[delimeter_position] << endl;

            //starting position of <offset, lenght of record>
            delimeter_position += 1;
            //while there's still data to read

            //OG
            // int64_t first;
            // int64_t second;

            //DEBUG, hard code fix?
            int64_t first;
            int64_t second;
            
            //cout << delimeter_position + (sizeof(int64_t) * 2) <= bytes_read << endl;
            //cout << second == 0 << endl;
            while(delimeter_position + (sizeof(int64_t) * 2) <= bytes_read){
                //read the next 2 integers
                memcpy(&first,  page_data + delimeter_position, 8);
                memcpy(&second, page_data + delimeter_position + 8, 8);
                

                //populate the slot_directory and the records
                slot_directory.push_back(make_pair(first, second));
                // cout << "reading" << "("<<first<<","<<second<<")"<< endl;
                // cout << "value of delimiter position:"<< delimeter_position<<endl;
                if(second == 0){
                    break;
                }

                //try to read the next ones
                delimeter_position += sizeof(int64_t) * 2;
                
            }

            // cout << "SHOULD print pairs " << slot_directory.size()<< endl;
            int c =1;
            for(auto const& slot: slot_directory){
                // cout << c <<"--slot pair" << "("<<slot.first<<","<<slot.second<<")"<< endl;
                c++;
            }
                
            // TO_DO: You may modify this function to process the search for employee ID in the page you just loaded to main memory.
            int cursor = 0;
            int num_record = 0;
            while(cursor < bytes_read && num_record < slot_directory.size()){
                //get a pointer to the data based on the offset
                const char* raw_data = &page_data[cursor];

                // read first 8 byte int - emplotee id
                string employee_id(raw_data, cursor+8 - cursor);
                //memcpy(&employee_id, raw_data, 8);
                raw_data += 8;
                cursor += 8;
                
                // cout << "read ID:" << employee_id << endl;
                
                char delimiter = *raw_data;
                //cout << "delimiter:"<< delimiter <<endl;
                if (delimiter != '$') {
                    
                    //throw runtime_error("Format error: expected '$' after employee_id");
                }
                raw_data++;
                cursor++;

                // Read name until '$'
                const char* name_start = raw_data;
                while (cursor < bytes_read && *raw_data != '$') {
                    raw_data++;
                    cursor++;
                }
                string name(name_start, raw_data - name_start);
                //cout << "read name: " << name << endl;
                delimiter = *raw_data;
                //cout << "delimter after name:"<< delimiter <<endl;

                raw_data++;  // Skip '$'
                cursor++;

                //read second string until '$' - bio
                // Read bio until '$'
                const char* bio_start = raw_data;
                while (cursor < bytes_read && *raw_data != '$') {
                    raw_data++;
                    cursor++;
                }
                string bio(bio_start, raw_data - bio_start);
                //cout << "read bio: " << endl<< bio <<endl;;
                delimiter = *raw_data;
                //cout << "character after bio:"<< delimiter <<endl;

                raw_data++;  // Skip '$'
                cursor++;

                //read last int64 - manager id
                string manager_id(raw_data, cursor+8 - cursor);
                //memcpy(&manager_id, raw_data, sizeof(manager_id));
                cursor += 8;
                raw_data += 8;


                //create a string vector with all the items
                vector<string> fields;
                fields.push_back(employee_id);
                fields.push_back(name);
                fields.push_back(bio);
                fields.push_back(manager_id);

                //create a record instant and populate the record vector
                Record my_record(fields);
                records.push_back(my_record);
                num_record++;

                if(*raw_data == '#'){
                    // cout << "have we found:"<<*raw_data << endl;
                    break;
                }
            }
            // cout << "SHOULD print RECORDs " << records.size()<< endl;
                c =1;
                for(auto const& record: records){
                    // cout << c <<"--record ID"<<record.id << endl;
                    c++;
                }

            return true;
        }

        if (bytes_read > 0) { 
            cerr << "Incomplete read: Expected " << 4096 << " bytes, but only read " << bytes_read << " bytes." << endl;
        }

        return false;
    }




    
};

class StorageManager {

public:
    string filename;  // Name of the file (EmployeeRelation.dat) where we will store the Pages 
    fstream data_file; // fstream to handle both input and output binary file operations
    vector <page> buffer; // You can have maximum of 3 Pages.
    
    // Constructor that opens a data file for binary input/output; truncates any existing data file
    StorageManager(const string& filename) : filename(filename) {
        data_file.open(filename, ios::binary | ios::out | ios::in | ios::trunc);
        if (!data_file.is_open()) {  // Check if the data_file was successfully opened
            cerr << "Failed to open data_file: " << filename << endl;
            exit(EXIT_FAILURE);  // Exit if the data_file cannot be opened
        }
    }

    // Destructor closes the data file if it is still open
    ~StorageManager() {
        if (data_file.is_open()) {
            data_file.close();
        }
    }

    // Reads data from a CSV file and writes it to EmployeeRelation.dat
    void createFromFile(const string& csvFilename) {
        buffer.resize(3); // You can have maximum of 3 Pages.

        ifstream csvFile(csvFilename);  // Open the Employee.csv file for reading
        
        string line, name, bio;
        int id, manager_id;
        int page_number = 0; // Current page we are working on [at most 3 pages]
        
        while (getline(csvFile, line)) {   // Read each line from the CSV file, parse it, and create Employee objects
            stringstream ss(line);
            string item;
            vector<string> fields;

            while (getline(ss, item, ',')) {
                fields.push_back(item);
            }
            Record r = Record(fields);  //create a record object   
            
            if (!buffer[page_number].insert_record_into_page(r)) { // inserting that record object to the current page
                
                // Current page is full, move to the next page
                page_number++;
 
                if (page_number >= buffer.size()) {    // Checking if page limit has been reached.
                    
                    for (page& p : buffer) { // using write_into_data_file() to write the pages into the data file
                        
                        p.write_into_data_file(data_file);
                    }
                    page_number = 0; // Starting again from page 0

                }
                buffer[page_number].insert_record_into_page(r); // Reattempting the insertion of record 'r' into the newly created page
            }
            
        }
        csvFile.close();  // Close the CSV file
    }

    // Searches for an Employee ID in EmployeeRelation.dat
    void findAndPrintEmployee(int searchId) {
        data_file.seekg(0, ios::beg);  // Rewind the data_file to the beginning for reading
        char page_data[4096] = {0};

        // TO_DO: Read pages from your data file (using read_from_data_file) and search for the employee ID in those pages. Be mindful of the page limit in main memory. 4KB        
        int page_number = 0;
        while(buffer[page_number].read_from_data_file(data_file)){ //deleted
            cout << "------------------------" << endl;
            cout << "---FINISHED READING PAGE #" << page_number + 1<< page_number+ 1<< page_number+ 1 << endl;
            cout << "------------------------" << endl;
            //get the first pair <offset, lenght>
            int num_record = 0;
            cout << "==== number of record: " <<buffer[page_number].records.size() << endl;
            cout << "==== number of slots: " <<buffer[page_number].slot_directory.size() << endl;
            int cursor = 0;
            
            for(auto &record : buffer[page_number].records){
                int employee_id = record.id;

                if(employee_id == searchId){
                    cout << "--------FOUND RECORD-------" << endl;
                    record.print();
                }
                num_record++;
            }    
            
            page_number++; //next page
            if(page_number > 2){
                break;
            }
        }
            cout << "MADE IT HERE" << endl;

        
        // TO_DO: Print "Record not found" if no records match.
        cout << "Record not found for the ID:" <<  searchId << endl;
        return;

        
    }
};
