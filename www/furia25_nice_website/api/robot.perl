#!/usr/bin/perl
# -----------------------------------------------------
# robot.perl — CGI profile-picture generator
#
# Modes:
#   ?name=foo              → HTML page (original)
#   ?name=foo&format=json  → JSON { avatar_url, set, verdict }
#
# Used by the Liminal Registry to generate robot avatars
# for authenticated users.
# -----------------------------------------------------

use strict;
use warnings;

# ---- read query string ----
my $query = $ENV{QUERY_STRING} || "";

sub qs_param {
    my ($key, $qs) = @_;
    if ($qs =~ /(?:^|&)\Q$key\E=([^&]*)/) {
        my $v = $1;
        $v =~ s/\+/ /g;
        $v =~ s/%([0-9A-Fa-f]{2})/chr(hex($1))/eg;
        return $v;
    }
    return "";
}

my $name   = qs_param("name",   $query);
my $format = qs_param("format", $query);

# ---- sanitize ----
sub html_escape {
    my $s = shift;
    $s =~ s/&/&amp;/g;
    $s =~ s/</&lt;/g;
    $s =~ s/>/&gt;/g;
    $s =~ s/"/&quot;/g;
    return $s;
}
my $safe_name = html_escape($name);

# ---- url-encode ----
sub url_encode {
    my $s = shift;
    $s =~ s/([^A-Za-z0-9\-\._~])/sprintf("%%%02X", ord($1))/eg;
    return $s;
}
my $effective_name = ($name ne "") ? $name : "anonymous_unit_0001";
my $encoded_name   = url_encode($effective_name);

# ---- pick set from name length ----
my @sets = ("set1", "set2", "set3", "set4");
my $set  = $sets[ length($effective_name) % scalar(@sets) ];

my $avatar_url = "https://robohash.org/$encoded_name?set=$set&size=300x300";

# ---- verdicts ----
my @verdicts = (
    "Diagnostic : 73% plus sarcastique qu'un grille-pain moyen.",
    "Attention : cette unite a ete vue en train de comploter contre l'imprimante.",
    "Analyse terminee. On recommande de le debrancher, au cas ou.",
    "Ce robot a tente de syndiquer le frigo connecte. Respect.",
    "Niveau de menace : faible. Probabilite de vol de snacks : alarmante.",
    "Certifie 100% sans gluten et legerement condescendant.",
    "Cette unite se croit le heros de l'histoire. Ce n'est pas le cas.",
    "Autonomie : eternelle. Patience pour le small talk : nulle.",
    "Un (1) agenda cache detecte concernant le mot de passe WiFi du bureau.",
    "Compatibilite avec les humains : etonnamment correcte, tout compte fait.",
    "Ce robot a des opinions tres arretees sur tabs vs espaces. Tabs ont gagne.",
    "Attention : connu pour fredonner des musiques de comedies musicales en calculant pi.",
);
sub simple_hash {
    my $s = shift;
    my $h = 0;
    foreach my $c (split //, $s) {
        $h = ($h * 31 + ord($c)) % 1000000;
    }
    return $h;
}
my $idx     = (simple_hash($effective_name) + time()) % scalar(@verdicts);
my $verdict = $verdicts[$idx];

# ---- facts ----
my @facts = (
    "Le saviez-vous : les robots revent en binaire. Surtout de 0. Ils adorent les 0.",
    "Le saviez-vous : la couleur preferee de ce robot est #00FF00, evidemment.",
    "Le saviez-vous : 9 robots sur 10 pensent que les majuscules sont un appel a l'aide.",
    "Le saviez-vous : quelque part, un robot aspirateur juge votre sol en ce moment meme.",
    "Le saviez-vous : les robots ne dorment pas, ils defragmentent leurs sentiments.",
    "Le saviez-vous : ce robot a perdu un concours de regard contre un micro-ondes.",
);
my $fact = $facts[int(rand(scalar(@facts)))];

# ============================================================
# JSON mode — used by the authenticated profile panel
# ============================================================
if ($format eq "json") {
    # json_escape: handle the handful of chars that matter
    sub json_escape {
        my $s = shift;
        $s =~ s/\\/\\\\/g;
        $s =~ s/"/\\"/g;
        $s =~ s/\n/\\n/g;
        $s =~ s/\r/\\r/g;
        $s =~ s/\t/\\t/g;
        return $s;
    }
    my $je_url     = json_escape($avatar_url);
    my $je_set     = json_escape($set);
    my $je_verdict = json_escape($verdict);
    my $je_name    = json_escape($effective_name);

    print "Content-Type: application/json; charset=UTF-8\r\n";
    print "Access-Control-Allow-Origin: http://localhost:8080\r\n";
    print "Access-Control-Allow-Credentials: true\r\n";
    print "\r\n";
    print "{\"ok\":true,\"avatar_url\":\"$je_url\",\"set\":\"$je_set\",\"verdict\":\"$je_verdict\",\"name\":\"$je_name\"}";
    exit 0;
}

# ============================================================
# HTML mode — original standalone page
# ============================================================
print "Content-Type: text/html; charset=UTF-8\r\n";
print "\r\n";

print <<"HTML";
<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="UTF-8">
<title>Generateur de Jumeau Robot</title>
<style>
    body {
        font-family: 'Courier New', monospace;
        background: #0d1117;
        color: #c9d1d9;
        display: flex;
        flex-direction: column;
        align-items: center;
        padding: 40px 20px;
        text-align: center;
    }
    h1 { color: #58a6ff; text-shadow: 0 0 8px rgba(88,166,255,0.5); }
    .card {
        background: #161b22;
        border: 1px solid #30363d;
        border-radius: 12px;
        padding: 24px 32px;
        max-width: 420px;
        margin-top: 20px;
        box-shadow: 0 0 20px rgba(0,0,0,0.5);
    }
    img { border-radius: 8px; border: 2px solid #30363d; margin: 16px 0; background: #0d1117; }
    .verdict { font-size: 1.1em; color: #f0883e; margin-top: 12px; }
    .fact    { font-size: 0.85em; color: #8b949e; margin-top: 20px; font-style: italic; }
    form     { margin-top: 30px; }
    input[type="text"] {
        padding: 8px 12px; border-radius: 6px; border: 1px solid #30363d;
        background: #0d1117; color: #c9d1d9; font-family: inherit;
    }
    input[type="submit"] {
        padding: 8px 16px; border-radius: 6px; border: none;
        background: #238636; color: white; font-family: inherit;
        cursor: pointer; margin-left: 8px;
    }
    input[type="submit"]:hover { background: #2ea043; }
    .meta { font-size: 0.75em; color: #6e7681; margin-top: 30px; }
</style>
</head>
<body>

<h1>&gt;&gt; GENERATEUR DE JUMEAU ROBOT &lt;&lt;</h1>
<p>Propulse par un algorithme tres serieux et l'API RoboHash.</p>

<div class="card">
HTML

if ($name eq "") {
    print "    <p>Aucune designation fournie. Voici une unite generique.</p>\n";
} else {
    print "    <p>Designation : <strong>$safe_name</strong></p>\n";
}

print <<"HTML";
    <img src="$avatar_url" alt="Votre jumeau robot" width="300" height="300">
    <div class="verdict">$verdict</div>
    <div class="fact">$fact</div>
</div>

<form method="get" action="">
    <label for="name">Entrez votre nom (ou une peur, un animal, n'importe quoi) :</label><br>
    <input type="text" id="name" name="name" placeholder="ex: ketchup" value="$safe_name">
    <input type="submit" value="GENERER LE JUMEAU">
</form>

<div class="meta">
    Set utilise : $set
    &middot; Rechargez pour un nouveau verdict, meme robot a chaque fois.
</div>

</body>
</html>
HTML
