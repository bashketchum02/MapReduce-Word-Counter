import string
from collections import defaultdict
from operator import itemgetter

def count_words(files, min_length, max_length):
    word_counts = defaultdict(int)

    for file in files:
        with open(file, 'r') as f:
            for line in f:
                line = line.strip()
                words = line.split()

                for word in words:
                    word = word.translate(str.maketrans('', '', string.punctuation+ "’‘“”—"))
                    word = word.lower()

                    if not word.isdigit() and min_length <= len(word) <= max_length:
                        word_counts[word] += 1

    return word_counts

def report_word_counts(word_counts, order):
    sorted_counts = sorted(word_counts.items(), key=itemgetter(1), reverse=True)

    if order == 0:
        report_text = "Word Count Report (Alphabetical Order):\n"
    else:
        report_text = "Word Count Report (Number of Words Order):\n"

    for word, count in sorted_counts:
        report_text += f"{word}: {count}\n"

    return report_text

file_count = int(input("Enter the number of text files: "))
files = []
for i in range(file_count):
    file = input(f"Enter the path of text file {i+1}: ")
    files.append(file)

min_length = int(input("Enter the minimum length of words to consider: "))
max_length = int(input("Enter the maximum length of words to consider: "))

order = input("Enter 'a' for alphabetical order or 'n' for number of words order: ")
if order.lower() == 'a':
    order = 0
else:
    order = 1

word_counts = count_words(files, min_length, max_length)
report_text = report_word_counts(word_counts, order)

with open("output.txt", "w", encoding='utf-8-sig') as output_file:
    output_file.write(report_text)
