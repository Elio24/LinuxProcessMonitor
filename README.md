# LP25 — Moniteur de processus

Projet réalisé en équipe dans le cadre de l’UE LP25.

Projet en C inspiré de `htop`. Il affiche les processus Linux dans le terminal avec ncurses. En local, il récupère les informations dans `/proc`. Il peut aussi interroger une machine distante et passer d’une machine à l’autre dans la même interface.

## Lancer le programme

Il faut Linux, GCC, Make et les en-têtes ncurses. Sur Debian ou Ubuntu, ces outils sont disponibles dans les paquets `build-essential` et `libncurses-dev`.

```sh
make
./my_htop
```

Utilise **F1** pour afficher l’aide, **F4** pour chercher un processus et **F10** pour quitter. **F2** et **F3** permettent de changer de machine.

Pour afficher le nombre de processus détectés sans ouvrir l’interface, lance `./my_htop --dry-run`. Les options de connexion sont listées dans `./my_htop --help`. Le mode SSH utilise `ssh` et `sshpass`.
