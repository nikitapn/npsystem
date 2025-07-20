#include <iostream>

#include <gtest/gtest.h>

#include <nplib/utils/trie.hpp>

using TrieSet = nplib::trie::TrieSet;

TEST(
  Test, TrieInsertAndSearch)
{
    TrieSet trieSet;

    // Insert some words
    EXPECT_TRUE(trieSet.insert("hello"));
    EXPECT_TRUE(trieSet.insert("world"));
    EXPECT_TRUE(trieSet.insert("hell"));
    EXPECT_TRUE(trieSet.insert("heaven"));

    // Search for inserted words
    EXPECT_TRUE(trieSet.search("hello"));
    EXPECT_TRUE(trieSet.search("world"));
    EXPECT_TRUE(trieSet.search("hell"));
    EXPECT_TRUE(trieSet.search("heaven"));

    // Search for non-inserted words
    EXPECT_FALSE(trieSet.search("helloo"));
    EXPECT_FALSE(trieSet.search("worl"));
    EXPECT_FALSE(trieSet.search("heav"));
}

TEST(
  Test, TrieInsertAndSearchSpecialCharacters)
{
    TrieSet trieSet;

    // Insert some words
    EXPECT_TRUE(trieSet.insert(".-.hello"));
    EXPECT_TRUE(trieSet.insert("hello-world"));
    EXPECT_TRUE(trieSet.insert("...."));
    EXPECT_TRUE(trieSet.insert("___heaven"));

    // Search for inserted words
    EXPECT_TRUE(trieSet.search(".-.hello"));
    EXPECT_TRUE(trieSet.search("hello-world"));
    EXPECT_TRUE(trieSet.search("...."));
    EXPECT_TRUE(trieSet.search("___heaven"));

    // Search for non-inserted words
    EXPECT_FALSE(trieSet.search("helloo"));
    EXPECT_FALSE(trieSet.search("worl"));
    EXPECT_FALSE(trieSet.search("heav"));
}

TEST(
  Test, TrieInsertAndSearchEmptyString)
{
    TrieSet trieSet;

    // Insert an empty string
    EXPECT_FALSE(trieSet.insert(""));

    // Search for the empty string
    EXPECT_FALSE(trieSet.search(""));
    
    // Search for non-inserted words
    EXPECT_FALSE(trieSet.search("a"));
}

TEST(
  Test, TrieInsertAndSearchLongStrings)
{
    TrieSet trieSet;

    // Insert long strings
    std::string longString1(10000, 'a'); // 1000 'a's
    std::string longString2(10000, 'b'); // 1000 'b's

    EXPECT_TRUE(trieSet.insert(longString1));
    EXPECT_TRUE(trieSet.insert(longString2));

    // Search for long strings
    EXPECT_TRUE(trieSet.search(longString1));
    EXPECT_TRUE(trieSet.search(longString2));

    // Search for non-inserted long strings
    std::string nonInsertedString(999, 'a'); // 999 'a's
    EXPECT_FALSE(trieSet.search(nonInsertedString));
}

TEST(
  Test, TrieInsertAndSearchMixedPatterns)
{
    TrieSet trieSet;

    // Insert mixed patterns
    EXPECT_TRUE(trieSet.insert("abc"));
    EXPECT_TRUE(trieSet.insert("abcdef"));

    // Search for inserted patterns
    EXPECT_TRUE(trieSet.search("abc"));
    EXPECT_TRUE(trieSet.search("abcdef"));

    // Search for non-inserted patterns
    EXPECT_FALSE(trieSet.search("abcd"));
    EXPECT_FALSE(trieSet.search("ab"));
}

TEST(
  Test, TrieInsertAndSearchWildcards)
{
    TrieSet trieSet;

    // Insert wildcard patterns
    EXPECT_TRUE(trieSet.insert("*abc"));
    EXPECT_TRUE(trieSet.insert("*def"));
    EXPECT_TRUE(trieSet.insert("xyz"));

    // Search for wildcard patterns
    EXPECT_TRUE(trieSet.search("--_abc"));
    EXPECT_TRUE(trieSet.search("...def"));
    EXPECT_TRUE(trieSet.search("xyz"));

    // Search for non-inserted patterns
    EXPECT_FALSE(trieSet.search("ab"));
    EXPECT_FALSE(trieSet.search("xy"));
}