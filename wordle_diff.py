small_set = set(open("wordle_possibles.txt", "r").read().strip().split("\n"))
big_set = set(open("wordle-nyt-words-14855.txt", "r").read().strip().split("\n"))


assert(len(small_set - big_set) == 0)
new_set = big_set - small_set

with open("wordle_diff.txt", "w") as f:
    for val in new_set:
        f.write(val)
        f.write("\n")
