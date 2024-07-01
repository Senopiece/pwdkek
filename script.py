from datetime import timedelta
from math import log2
from transformers import GPT2LMHeadModel, RobertaTokenizerFast
import torch
import torch.nn.functional as F

NUM_GENERATIONS = 1
MIN_CHARS = 6
MAX_CHARS = 10

# Input a predefined password from the user
predefined_password = input("Enter a password: ")
assert len(predefined_password) >= MIN_CHARS, "Too short password"
assert len(predefined_password) <= MAX_CHARS, "Too long password"

tokenizer = RobertaTokenizerFast.from_pretrained(
    "javirandor/passgpt-10characters",
    model_max_length=MAX_CHARS + 2,  # Max length + start and end tokens
    padding="max_length",
    truncation=True,
    do_lower_case=False,
    strip_accents=False,
    mask_token="<mask>",
    unk_token="<unk>",
    pad_token="<pad>",
    truncation_side="right",
)

predefined_tokens = tokenizer(predefined_password, return_tensors="pt")["input_ids"][0][
    1:
]

model = GPT2LMHeadModel.from_pretrained("javirandor/passgpt-10characters").eval()

# Manually generate the sequence step-by-step based on predefined password
generated = torch.tensor([[tokenizer.bos_token_id]])

entropy = 0

for idx, token in enumerate(predefined_tokens):
    outputs = model(generated)
    next_token_logits = outputs.logits[:, -1, :]

    # Calculate probabilities
    probabilities = F.softmax(next_token_logits, dim=-1)

    # Choose the next token from the predefined password
    next_token = token.unsqueeze(0).unsqueeze(0)

    # Print the probability of the chosen token
    chosen_token_prob = probabilities[0, token].item()
    chosen_token_str = tokenizer.convert_ids_to_tokens([token.item()])[0]

    # Add to entropy
    try:
        entropy += -log2(chosen_token_prob)
    except ValueError:
        entropy += 10e10

    # Append the chosen token to the generated sequence
    generated = torch.cat((generated, next_token), dim=1)

print("Password entropy:", entropy)

tiers = ["Pathetic", "Low", "Medium", "High", "Extreme"]


def tier(ttd: timedelta):
    for i, tier in enumerate(tiers[:-1]):
        threshold = timedelta(days=pow(356, i))

        if ttd < threshold:
            return tier

    return tiers[-1]


ttd = timedelta(seconds=10e-9 * pow(2, entropy))
years = ttd.days // 365
days = ttd.days % 365
hours, remainder = divmod(ttd.seconds, 3600)
minutes, seconds = divmod(remainder, 60)
print(
    "Time to decode with 1Gh/s:",
    f"{years} years {days} days {hours} hours {minutes} minutes {seconds} seconds",
)
print("Tier:", tier(ttd))
