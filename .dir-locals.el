;;; Directory Local Variables            -*- no-byte-compile: t -*-
;;; For more information see (info "(emacs) Directory Variables")

((nil    . ((projectile-project-compilation-cmd . "make")
            (projectile-project-configure-cmd   . "make clean")
            (+license/license-choice            . "GPLv2")))
 (c-mode . ((flycheck-gcc-include-path          . (".."))
            (flycheck-clang-include-path        . (".."))
            (company-clang-arguments            . ("-I.."))
            (eval                               . (clang-format-mode t)))))
