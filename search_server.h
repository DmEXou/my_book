#pragma once
#include <algorithm>
#include <map>

#include <iostream>///

#include "document_status.h"
#include "string_processing.h"

using namespace std;///

class SearchServer {
public:
    template <typename StringContainer>
    SearchServer(const StringContainer& stop_words);

    SearchServer(const std::string& stop_words_text);

    void AddDocument(int document_id, const std::string& document,
        DocumentStatus status, const std::vector<int>& ratings);

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate predicate) const {
        const Query query = ParseQuery(raw_query);
        std::vector<Document> result_search;
        auto matched_documents = FindAllDocuments(query);
        for (auto& doc : matched_documents) {
            if (predicate(doc.id, documents_.at(doc.id).status, doc.rating) != false) result_search.push_back(doc);
        }
        std::sort(result_search.begin(), result_search.end(),
            [](const Document& lhs, const Document& rhs) {
                if (std::abs(lhs.relevance - rhs.relevance) < 1e-6) {
                    return lhs.rating > rhs.rating;
                }
                else {
                    return lhs.relevance > rhs.relevance;
                }
            });
        if (result_search.size() > MaxResultDocumentCount) {
            result_search.resize(MaxResultDocumentCount);
        }
        return result_search;
    }

    std::vector<Document> FindTopDocuments(const std::string& raw_query,
        DocumentStatus status = DocumentStatus::ACTUAL) const {
        auto CheckStatus = [status](int document_id, DocumentStatus status_, int rating) {
            return status_ == status;
        };
        return FindTopDocuments(raw_query, CheckStatus);
    }

    int GetDocumentCount() const;

    auto begin() const {
        return document_ids_.begin();
    }

    auto end() const {
        return document_ids_.end();
    }
    /////////////
    const std::map<string, double>& GetWordFrequencies(int document_id) const {
        std::map<string, double> freqs_word = {};
        int counter_all_str = 0;
        for (const auto& tmp_map : word_to_document_freqs_) {
            counter_all_str += tmp_map.second.size();
            for (const auto& second : tmp_map.second) {
                if (second.first == document_id) {
                    freqs_word[tmp_map.first] = tmp_map.second.size();
                }
            }
        }
        for (auto& freqs_word_elem : freqs_word) {
            freqs_word_elem.second /= counter_all_str;
        }
        return freqs_word;
        //O(logN);//�����, ��� ���������� ���� ����, �������� �� ����� ���������� ����.
    }
    /////////////
    void RemoveDocument(int document_id) {
        auto it = (document_ids_.begin() + document_id - 1);
        document_ids_.erase(it);
        auto itm = (documents_.begin()->first + document_id - 1);
        documents_.erase(itm);
    }
    ///O(WlogN)
    /////////////
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query,
        int document_id) const;
    
private:
    
    struct DocumentData {
        int rating = 0;
        DocumentStatus status = DocumentStatus::ACTUAL;
    };
    
    struct QueryWord {
        std::string data;
        bool is_minus = false;
        bool is_stop = false;
    };
    
    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

private:
    
    bool IsStopWord(const std::string& word) const;

    static bool IsValidWord(const std::string& word);

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;
    
    static int ComputeAverageRating(const std::vector<int>& ratings);
    
    QueryWord ParseQueryWord(std::string text) const;
    
    Query ParseQuery(const std::string& text) const;
    
    double ComputeWordInverseDocumentFreq(const std::string& word) const;
    
    std::vector<Document> FindAllDocuments(const Query& query) const;

private:
    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> document_ids_;
    
private:
    const int MaxResultDocumentCount = 5;
};
