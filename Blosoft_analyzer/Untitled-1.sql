-- database: c:\Users\enzod\Desktop\Programmation\Git\Blossom-Blosoft\Blosoft_analyzer\BlosoftDB__.db

-- Appuyez sur le bouton ▷ dans le coin supérieur droit de la fenêtre pour exécuter l'ensemble du fichier.

SELECT category_id, SUM(data_quantity) AS total_quantity
FROM data_entries
GROUP BY category_id;
