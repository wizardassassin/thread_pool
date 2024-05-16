score_data = open("wordle_scores.txt", "r").read().strip().split("\n")
matrix_data = open("wordle_matrix.txt", "r").read().strip().split("\n")

assert len(score_data) == len(matrix_data)

for score, matrix in zip(score_data, matrix_data):
    m_sum = sum([int(y) for y in matrix.strip().split(" ")])
    assert int(score) == m_sum
