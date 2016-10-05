
# -v:     print steps, print failure input
# -vv:    print keys and data
# -vvv:   print intermediate values
# -vvvv:  print decoder rounds
# -vvvvv: print decoder rounds values

def main():
    global args
    err_code = 0

    ap = argparse.ArgumentParser()
    ap.add_argument("-s", "--selftest", action="store_true",
                    help="run internal consistency tests")
    ap.add_argument("-p", "--testpack", action="store_true",
                    help="run internal pack/unpack tests")
    ap.add_argument("-g", "--generate", action="store_true",
                    help="generate test vectors")
    ap.add_argument("-k", "--keys", type=int, default=1,
                    help="number of random keys to be tested (default: 5)")
    ap.add_argument("-m", "--messages", type=int, default=2,
                    help="number of random messages to be tested (default: 5)")
    ap.add_argument("-d", "--data", action="store_true",
                    help="provide data from stdin (default: random data)")
    ap.add_argument("-v", "--verbose", action="count", default=0,
                    help="increase output verbosity")
    args = ap.parse_args()
    if args.selftest:
        test = Test(2, 4801, 45, 84, [29, 27, 25, 24, 23, 23])
        if args.data:
            data = Util.parse_input_selftest(4801);
            err_code |= test.self_test_correctness(data);
        else:
            err_code |= test.self_test_correctness((args.keys, args.messages, None))
    if args.testpack:
        err_code |= test.self_test_pack_unpack()
    if args.generate:
        gen_test_vectors(args.keys, args.messages)

    if err_code:
        sys.exit(err_code)


if __name__ == "__main__":
    main()

    try:
        get_ipython()
    except NameError:
        # only run automatically when not in the repl
        main()
    else:
        # set globals as helper variables in the repl
        global gN
        global gKD
        global gT
        gN = Niederreiter(2, 4801, 45, 84, [29, 27, 25, 24, 23, 23])
        gKD = KEMDEM(2, 4801, 45, 84, [29, 27, 25, 24, 23, 23])
        gT = Test(2, 4801, 45, 84, [29, 27, 25, 24, 23, 23])
        # gN = Niederreiter(2, 71, 3, 4, [3, 2, 2, 1, 1, 1])
        # gKD = KEMDEM(2, 71, 3, 4, [3, 2, 2, 1, 1, 1])
        # gT = Test(2, 71, 3, 4, [3, 2, 2, 1, 1, 1])
