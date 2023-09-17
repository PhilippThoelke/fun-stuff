import time

import cv2
import torch
from torch import nn


class CPPN(nn.Module):
    def __init__(
        self,
        *funcs,
        layers: int = 4,
        hidden_dim: int = 16,
        height: int = 512,
        width: int = 512,
        scale: float = 250
    ) -> None:
        super().__init__()

        self.height = height
        self.width = width
        self.scale = scale

        assert len(funcs) > 0, "At least one function must be provided"
        self.funcs = funcs

        self.before = nn.Linear(3, hidden_dim)
        self.layers = nn.ModuleList(
            [nn.Linear(hidden_dim * len(funcs), hidden_dim) for _ in range(layers)]
        )
        self.after = nn.Linear(hidden_dim, 3)

        self.reset_parameters()

    def reset_parameters(self):
        self.before.reset_parameters()
        for layer in self.layers:
            layer.reset_parameters()
            nn.init.normal_(layer.bias.data)
        self.after.reset_parameters()

    def forward(self, zoom: float, t: float):
        device = self.before.weight.device
        x = torch.linspace(-1, 1, self.width, device=device) * self.scale * zoom
        y = torch.linspace(-1, 1, self.height, device=device) * self.scale * zoom
        x, y = torch.meshgrid(x, y, indexing="ij")

        z = torch.stack([x, y, torch.ones_like(x) * t], dim=-1)
        z = self.before(z)
        for layer in self.layers:
            zs = torch.cat([func(z) for func in self.funcs], dim=-1)
            z = layer(zs)
        z = self.after(z)
        z = torch.sigmoid(z)
        vmin = z.reshape(-1, 3).min(dim=0).values
        vmax = z.reshape(-1, 3).max(dim=0).values
        z = (z - vmin) / (vmax - vmin)
        return z**2


def gaussian(x):
    return torch.exp(-(x**2))


step = 0
n = 400

cppn = CPPN(torch.sin, torch.cos, gaussian, torch.abs, torch.sigmoid, torch.relu)
cppn.to("cuda")
last = time.time()
while True:
    step = step % n
    if step == 0 or cppn is None:
        cppn.reset_parameters()

    zoom = 2 - (step / n)
    t = step / 2 + 10

    img = cppn(zoom, t).detach().cpu().numpy()
    cv2.imshow("img", cv2.cvtColor(img, cv2.COLOR_RGB2BGR))
    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

    sleep_time = 1 / 30 - (time.time() - last)
    if sleep_time > 0:
        time.sleep(sleep_time)
    last = time.time()

    step += 1
