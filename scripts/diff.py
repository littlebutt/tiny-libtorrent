import sys


def diff_str(a: str, b: str):
    length = len(a) if len(a) < len(b) else len(b)
    for i in range(length):
        if a[i] != b[i]:
            print(f"The two strs are different at {i} with the char {a[i]}.")
    else:
        print("The two strings are identical.")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        exit(-1)
    f = open(sys.argv[1], "r")
    str1 = f.read()
    f.close()

    f = open(sys.argv[2], "r")
    str2 = f.read()
    f.close()
    diff_str(str1, str2)