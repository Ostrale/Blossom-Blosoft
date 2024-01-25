# Format de la base de données

### Categories
| id | category  |
|----|-----------|
| 1  | Category1 |
| 2  | Category2 |
| 3  | Category3 |

#### Breeds
| id | Breed  |
|----|--------|
| 1  | Breed1 |
| 2  | Breed2 |
| 3  | Breed3 |

#### Protocols
| id | Protocol  | category_id | Breed_id |
|----|-----------|-------------|----------|
| 1  | Protocol1 | 1           | 2        |
| 2  | Protocol2 | 1           | 2        |
| 3  | Protocol3 | 3           | 3        |

#### data_entries
| id | Protocol_id | timestamp   | data_quantity |
|----|--------------|-------------|---------------|
| 1  | 1            | 123456789   | 10            |
| 2  | 2            | 123456790   | 15            |
| 3  | 1            | 123456791   | 8             |
| 4  | 3            | 123456792   | 20            |
