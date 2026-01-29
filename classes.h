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

        /*ostringstream oss;
        oss.write(reinterpret_cast<const char*>(&id), sizeof(id)); // Writes the binary representation of the ID.
        oss.write(&dollar, sizeof(dollar));// (new!) Writes the binary representation of a $
        oss.write(reinterpret_cast<const char*>(&manager_id), sizeof(manager_id)); // Writes the binary representation of the Manager id
        oss.write(&dollar, sizeof(dollar));// (new!) Writes the binary representation of a $
        int name_len = name.size();
        int bio_len = bio.size();
        oss.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len)); // // Writes the size of the Name in binary format.
        oss.write(&dollar, sizeof(dollar));// (new!) Writes the binary representation of a $
        oss.write(name.c_str(), name.size()); // writes the name in binary form
        oss.write(&dollar, sizeof(dollar));// (new!) Writes the binary representation of a $
        oss.write(reinterpret_cast<const char*>(&bio_len), sizeof(bio_len)); // // Writes the size of the Bio in binary format. 
        oss.write(&dollar, sizeof(dollar));// (new!) Writes the binary representation of a $
        oss.write(bio.c_str(), bio.size()); // writes bio in binary form
        oss.write(&dollar, sizeof(dollar));// (new!) Writes the binary representation of a $*/
        return to_string(id)+ dollar + name + dollar + bio + dollar + to_string(manager_id) + dollar;
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
        int slot_size = sizeof(int) * 2;
        if (cur_size + record_size + slot_size > 4096) { //Check if page size limit exceeded, considering slot directory size
            return false; // Cannot insert the record into this page
        } else {
            records.push_back(r); // Record stored in current page
            // TO_DO: update slot directory information
            slot_directory.push_back(make_pair(cur_size, record_size));
            //DEBUGGIN
            //cout << "INERTING:" << "(" << cur_size << "," << record_size << ")" << endl;
            cur_size += r.get_size(); // Updating page size
            return true;
        }
        
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
            
            //DEBUGGING
            /*
            cout << "********************************" << endl;
            cout << "copint to memory:" << first << "&&"<< second << "---" << endl;
            cout << "********************************" << endl;
            */
            
        }
        
        out.write(page_data, sizeof(page_data)); // Write the page_data to the EmployeeRelation.dat file 

    }

    // Read a page from a binary input stream, i.e., EmployeeRelation.dat file to populate a page object
    bool read_from_data_file(istream& in) {
        char page_data[4096] = {0}; // Character array used to read 4 KB from the data file to your main memory. 
        in.read(page_data, 4096); // Read a page of 4 KB from the data file 

        streamsize bytes_read = in.gcount(); // used to check if 4KB was actually read from the data file
        if (bytes_read == 4096) {
            
            // TO_DO: You may process page_data (4 KB page) and put the information to the records and slot_directory (main memory).
            // read page_data backward from footer until we find '#'
            size_t delimeter_position = 4097;
            for (size_t i = bytes_read - 1; i >= 0; i-- > 0) {
                //cout << page_data[i] << ",";
                //find the record delimeter. - #
                if(page_data[i] == '#'){
                    delimeter_position = i;
                    break;
                }
            }

            //DEBUGGIN
            //char my_char;
            //memcpy(&my_char,  page_data + delimeter_position, 1);
            //cout << my_char << endl;
            //out << "START HERE\n";
            

            //starting position of <offset, lenght of record>
            delimeter_position += 1;
            //while there's still data to read
            int64_t first;
            int64_t second;
            int limit = 0;
            while(delimeter_position + sizeof(int64_t) * 2 <= bytes_read && limit <10){
                //read the next 2 integers
                memcpy(&first,  page_data + delimeter_position, 8);
                memcpy(&second, page_data + delimeter_position + 8, 8);

                //DEBUGGIN
                //cout << "@@@" <<first << "--" << second<< "@@@" << endl;

                //populate the slot_directory and the records
                slot_directory.push_back(make_pair(first, second));

                //try to read the next ones
                delimeter_position += sizeof(int64_t) * 2;
                limit++;   
            }
            //cout << "END HERE" << endl;
            // TO_DO: You may modify this function to process the search for employee ID in the page you just loaded to main memory.
            //read the first record
            int start = 0;
            int end = -1;
            end = slot_directory[0].second;
            for(const auto& slots : slot_directory){
                //read record into char*
                char* my_string;
                strncpy(my_string, page_data + start, end);
                
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

        // TO_DO: Read pages from your data file (using read_from_data_file) and search for the employee ID in those pages. Be mindful of the page limit in main memory. 4KB        
        int page_number = 0;
        buffer[page_number].read_from_data_file(data_file);
        for(int i=0; buffer[page_number].read_from_data_file(data_file) && i<3; i++){
            if(false){
                cout << "Record found for the ID:" <<  searchId << endl;
                cout << "print here all the info for the ID";
            }
        }
        
        // TO_DO: Print "Record not found" if no records match.
        cout << "Record not found for the ID:" <<  searchId << endl;
        return;

    }
};
