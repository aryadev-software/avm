;;; Directory Local Variables            -*- no-byte-compile: t -*-
;;; For more information see (info "(emacs) Directory Variables")

((nil    . ((projectile-project-compilation-cmd . "make")
            (projectile-project-configure-cmd   . "make clean")
            (projectile-project-test-cmd        . "make test")
            (+license/license-choice            . "GPLv2")))
 (c-mode . ((mode . clang-format)
            (eval . (eglot-ensure)))))
