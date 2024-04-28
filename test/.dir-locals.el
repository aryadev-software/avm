;;; Directory Local Variables         -*- no-byte-compile: t; -*-
;;; For more information see (info "(emacs) Directory Variables")

((nil    . ((+license/license-choice            . "GPLv2")))
 (c-mode . ((flycheck-gcc-include-path          . (".." "../.."))
            (flycheck-clang-include-path        . (".." "../.."))
            (company-clang-arguments            . ("-I.." "-I../../"))
            (eval                               . (clang-format-mode t)))))
