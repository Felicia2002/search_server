#pragma once
 
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <map>
#include <set>
#include <deque>
#include <cmath>
#include <iostream>
#include <execution>
#include <type_traits>
 
#include "document.h"
#include "concurrent_map.h"
#include "string_processing.h"
 
using namespace std::string_literals;
using namespace std::string_view_literals;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double MIN_REAL_VALUE = 1e-6;

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);

    void AddDocument(int document_id,
                     const std::string_view document,
                     DocumentStatus status,
                     const std::vector<int>& ratings);

    int GetDocumentCount() const;

    std::vector<int>::const_iterator begin() const;

    std::vector<int>::const_iterator end() const;

    bool CompareDocumentsWords(int id1, int id2) const;

    const std::map<std::string_view, double>&
    GetWordFrequencies(int document_id) const;

// RemoveDocument
    void RemoveDocument(int document_id);

    void RemoveDocument(const std::execution::sequenced_policy& policy,
                        int document_id);

    void RemoveDocument(const std::execution::parallel_policy& policy,
                        int document_id);

// FindTopDocuments

    template <typename Predicate>
    std::vector<Document>
    FindTopDocuments(const std::string_view raw_query,
                     Predicate document_predicate) const;

    std::vector<Document>
    FindTopDocuments(const std::string_view raw_query,
                     DocumentStatus status) const;

    std::vector<Document>
    FindTopDocuments(const std::string_view raw_query) const;

    template <typename ExecutionPolicy, typename Predicate>
    std::vector<Document>
    FindTopDocuments(const ExecutionPolicy& policy,
                     const std::string_view raw_query,
                     Predicate document_predicate) const;

    template <typename ExecutionPolicy>
    std::vector<Document>
    FindTopDocuments(const ExecutionPolicy& policy,
                     const std::string_view raw_query,
                     DocumentStatus status) const;

    template <typename ExecutionPolicy>
    std::vector<Document>
    FindTopDocuments(const ExecutionPolicy& policy,
                     const std::string_view raw_query) const;

// MatchDocument
    std::tuple<std::vector<std::string_view>, DocumentStatus>
    MatchDocument(const std::string_view raw_query, int document_id) const;

    std::tuple<std::vector<std::string_view>, DocumentStatus>
    MatchDocument(const std::execution::sequenced_policy& policy,
                  const std::string_view raw_query, int document_id) const;

    std::tuple<std::vector<std::string_view>, DocumentStatus>
    MatchDocument(const std::execution::parallel_policy& policy,
                  const std::string_view raw_query, int document_id) const;

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    struct Query {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;
    };

    const std::set<std::string, std::less<>> stop_words_;
    std::set<std::string, std::less<>> words_;

    std::map<std::string_view, std::map<int, double>>
    word_to_document_freqs_;
    std::map<int, std::map<std::string_view, double>>
    document_to_word_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> document_ids_;

    bool IsStopWord(const std::string_view word) const;

    static bool IsValidWord(const std::string_view word);

    static bool IsNonValidWord(const std::string_view word);

    std::vector<std::string_view>
    SplitIntoWordsNoStop(const std::string_view text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    // Existence required
    double ComputeWordInverseDocumentFreq(
           const std::string_view word) const;

    QueryWord ParseQueryWord(const std::string_view text) const;

// ParseQuery
    Query ParseQuery(const std::string_view text) const;

    Query ParseQuery(const std::execution::sequenced_policy& policy,
                     const std::string_view text) const;

    Query ParseQuery(const std::execution::parallel_policy& policy,
                     const std::string_view text) const;

// FindAllDocuments
    template <typename Predicate>
    std::vector<Document>
    FindAllDocuments(const Query& query,
                     Predicate document_predicate) const;

    template <typename Predicate>
    std::vector<Document>
    FindAllDocuments(const std::execution::sequenced_policy& policy,
                     const Query& query,
                     Predicate document_predicate) const;

    template <typename Predicate>
    std::vector<Document>
    FindAllDocuments(const std::execution::parallel_policy& policy,
                     const Query& query,
                     Predicate document_predicate) const;
};

// PUBLIC

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words))
{
    if (!all_of(stop_words_.begin(), stop_words_.end(),
                IsValidWord)) {
               throw std::invalid_argument(
                          "Some of stop words are invalid"s);
    }
}

// FindTopDocuments

template <typename Predicate>
std::vector<Document>
SearchServer::FindTopDocuments(
              const std::string_view raw_query,
              Predicate document_predicate) const {
    const auto query = ParseQuery(raw_query);

    auto matched_documents = FindAllDocuments(query,
                                              document_predicate);

    std::sort(matched_documents.begin(),
              matched_documents.end(),
              [](const Document& lhs, const Document& rhs) {
                  if (std::abs(lhs.relevance - rhs.relevance) <
                      MIN_REAL_VALUE) {
                      return lhs.rating > rhs.rating;
                  } else {
                      return lhs.relevance > rhs.relevance;
                  }
              });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename ExecutionPolicy, typename Predicate>
std::vector<Document>
SearchServer::FindTopDocuments(
              const ExecutionPolicy& policy,          
              const std::string_view raw_query,
              Predicate document_predicate) const {

    const Query query = ParseQuery(/*std::execution::seq,*/ raw_query);

    std::vector<Document>
    matched_documents = FindAllDocuments(policy, query,
                                         document_predicate);
 
    std::sort(policy, matched_documents.begin(),
              matched_documents.end(),
              [](const Document& lhs, const Document& rhs) {
                  if (std::abs(lhs.relevance - rhs.relevance) <
                      MIN_REAL_VALUE) {
                      return lhs.rating > rhs.rating;
                  } else {
                      return lhs.relevance > rhs.relevance;
                  }
              });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename ExecutionPolicy>
std::vector<Document>
SearchServer::FindTopDocuments(
              const ExecutionPolicy& policy,
              const std::string_view raw_query,
              DocumentStatus status) const {
    return FindTopDocuments(policy, raw_query,
           [status](int document_id,
                    DocumentStatus document_status,
                    int rating) {
                        return document_status == status;
                    });
}

template <typename ExecutionPolicy>
std::vector<Document>
SearchServer::FindTopDocuments(
              const ExecutionPolicy& policy,
              const std::string_view raw_query) const {
    return FindTopDocuments(policy, raw_query,
                            DocumentStatus::ACTUAL);
}

// PRIVATE

// FindAllDocuments
template <typename Predicate>
std::vector<Document>
SearchServer::FindAllDocuments(
              const Query& query,
              Predicate document_predicate) const {

    std::map<int, double> document_to_relevance;

    for (const std::string_view word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }

        const double inverse_document_freq =
                     ComputeWordInverseDocumentFreq(word);

        for (const auto [document_id, term_freq] :
             word_to_document_freqs_.at(word)) {
            const auto& document_data =
                documents_.at(document_id);
            if (document_predicate(document_id,
                document_data.status,
                document_data.rating)) {
                document_to_relevance[document_id] +=
                    term_freq * inverse_document_freq;
            }
        }
    }

    for (const std::string_view word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }

        for (const auto [document_id, _] :
            word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] :
         document_to_relevance) {
        matched_documents.push_back(
            { document_id, relevance,
              documents_.at(document_id).rating });
        }

    return matched_documents;
}

// FindAllDocuments sequenced_policy
template <typename Predicate>
std::vector<Document>
SearchServer::FindAllDocuments(
              const std::execution::sequenced_policy& policy,
              const Query& query,
              Predicate document_predicate) const {
    return FindAllDocuments(query, document_predicate);
}

// FindAllDocuments parallel_policy
template <typename Predicate>
std::vector<Document>
SearchServer::FindAllDocuments(
              const std::execution::parallel_policy& policy,
              const Query& query,
              Predicate document_predicate) const {

    ConcurrentMap<int, double> relevances(101);

    for_each (policy,
              query.plus_words.begin(),
              query.plus_words.end(),
              [this, &relevances, &document_predicate]
              (std::string_view word) {
                  if (word_to_document_freqs_.count(word) == 0) {
                      return;
                  }
                  const double
                  idf = ComputeWordInverseDocumentFreq(word);

                  for (const auto& [id, freq]
                       : word_to_document_freqs_.at(word)) {
                      const DocumentData doc = documents_.at(id);
                      if (document_predicate(id, doc.status,
                                             doc.rating)) {
                          relevances[id].ref_to_value +=
                              freq * idf;
                      }
                  }
              });
              
    for_each (policy,
              query.minus_words.begin(),
              query.minus_words.end(),
              [this, &relevances]
              (std::string_view word) {
                  if (word_to_document_freqs_.count(word) == 0) {
                      return;
                  }

                  for (const auto& [id, _]
                       : word_to_document_freqs_.at(word)) {
                      relevances.Erase(id);
                  }
              });

    std::vector<Document> matched_documents;
    for (const auto [id, relevance] :
         relevances.BuildOrdinaryMap()) {
        matched_documents.push_back(
            { id, relevance,
              documents_.at(id).rating });
    }

    return matched_documents;
}