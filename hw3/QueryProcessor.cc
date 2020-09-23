/*
 * Copyright ©2020 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Summer Quarter 2020 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include "./QueryProcessor.h"

#include <iostream>
#include <algorithm>

extern "C" {
  #include "./libhw1/CSE333.h"
}

using std::list;
using std::sort;
using std::string;
using std::vector;

namespace hw3 {

// helper function,take the query of type vector<string>, a doctable_reader,
// and a index_reader to process one query, return the resulting vector of type
// QueryResult
static vector<QueryProcessor::QueryResult> ProcessOne(const vector<string>
        &query,
        DocTableReader* doctable_reader,
        IndexTableReader *index_reader);

QueryProcessor::QueryProcessor(const list<string> &indexlist, bool validate) {
  // Stash away a copy of the index list.
  indexlist_ = indexlist;
  arraylen_ = indexlist_.size();
  Verify333(arraylen_ > 0);

  // Create the arrays of DocTableReader*'s. and IndexTableReader*'s.
  dtr_array_ = new DocTableReader *[arraylen_];
  itr_array_ = new IndexTableReader *[arraylen_];

  // Populate the arrays with heap-allocated DocTableReader and
  // IndexTableReader object instances.
  list<string>::const_iterator idx_iterator = indexlist_.begin();
  for (int i = 0; i < arraylen_; i++) {
    FileIndexReader fir(*idx_iterator, validate);
    dtr_array_[i] = fir.NewDocTableReader();
    itr_array_[i] = fir.NewIndexTableReader();
    idx_iterator++;
  }
}

QueryProcessor::~QueryProcessor() {
  // Delete the heap-allocated DocTableReader and IndexTableReader
  // object instances.
  Verify333(dtr_array_ != nullptr);
  Verify333(itr_array_ != nullptr);
  for (int i = 0; i < arraylen_; i++) {
    delete dtr_array_[i];
    delete itr_array_[i];
  }

  // Delete the arrays of DocTableReader*'s and IndexTableReader*'s.
  delete[] dtr_array_;
  delete[] itr_array_;
  dtr_array_ = nullptr;
  itr_array_ = nullptr;
}

// This structure is used to store a index-file-specific query result.
typedef struct {
  DocID_t docid;  // The document ID within the index file.
  int rank;       // The rank of the result so far.
} IdxQueryResult;



vector<QueryProcessor::QueryResult>
QueryProcessor::ProcessQuery(const vector<string> &query) {
  Verify333(query.size() > 0);

  // STEP 1.
  // (the only step in this file)
  vector<QueryProcessor::QueryResult> finalresult;

  for (int i = 0; i < arraylen_; i++) {
      vector<QueryProcessor::QueryResult> result;
      IndexTableReader *index_reader = itr_array_[i];
      DocTableReader *doctable_reader = dtr_array_[i];
      result = ProcessOne(query, doctable_reader, index_reader);
      // append the result
      finalresult.insert(finalresult.end(), result.begin(), result.end());
  }


  // Sort the final results.
  sort(finalresult.begin(), finalresult.end());
  return finalresult;
}

static vector<QueryProcessor::QueryResult> ProcessOne(const vector<string>
        &query,
        DocTableReader* doctable_reader,
        IndexTableReader *index_reader) {
    vector<QueryProcessor::QueryResult> ret_list;
    // look up the first word in the query
    DocIDTableReader *first_docID_reader = index_reader->LookupWord(query[0]);
    if (first_docID_reader == nullptr) {
        delete first_docID_reader;
        return ret_list;
    }
    list<DocIDElementHeader> res = first_docID_reader->GetDocIDList();
    delete first_docID_reader;
    list<DocPositionOffset_t> ret_val;
    // look up the rest of words inside the query
    // trim the res list or update res's rank
    for (uint8_t i = 1; i < query.size(); ++i) {
        DocIDTableReader *docID_reader = index_reader->LookupWord
                (query[i]);
        if (docID_reader == nullptr) {
            delete docID_reader;
            return ret_list;
        }
        for (auto it = res.begin(); it != res.end(); it++) {
          // if we can find the docID inside res, update the rank
            if (docID_reader->LookupDocID(it->docID, &ret_val)) {
                it->numPositions += ret_val.size();
            } else {
              // if we can't find, trim it
                it = res.erase(it);
                it--;
            }
        }
        delete docID_reader;
    }
    // convert docID into filename, complete the QueryResult struct, append
    // to the ret_list
    string filename;
    for (auto it = res.begin(); it != res.end(); it++) {
        Verify333(doctable_reader->LookupDocID(it->docID, &filename));
        ret_list.push_back({filename, it->numPositions});
    }
    return ret_list;
}


}  // namespace hw3
