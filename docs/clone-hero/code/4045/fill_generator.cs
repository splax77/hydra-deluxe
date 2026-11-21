// Clone Hero v1.0.0.4045 (Mono build), decompiled with ilspycmd 11.0.0.9375 from
// C:\Clone Hero\1.0 old\Clone Hero\Clone Hero_Data\Managed\Assembly-CSharp.dll on 2026-10-30.
// F6 to F13 were added on 2026-11-21 from ilspycmd 11.0.0.9375 -p output of the same Assembly-CSharp.dll,
// of StrikeCore.dll (F12) and, with -t UnityEngine.Mathf, of UnityEngine.CoreModule.dll (F13), all in that folder.
// Only the relevant methods are kept, copied unchanged. Names are the game's scrambled names.
// Line numbers in each banner are lines of the full decompiled file, not of this file.

// ---- caller header: static class ʶˀʵˁʹʿˀʶʿʲʾ method ʹʹʾʴʳʶʼʵʳʼʼ (start) : ʶˀʵˁʹʿˀʶʿʲʾ.cs lines 823 to 833 of the ilspycmd output ----
	public static ʹʲʼʺʶʷʾʶʿʺʹ ʹʹʾʴʳʶʼʵʳʼʼ(BasePlayer ʾʵʻʳʷʴʼʲʻʶʷ, ʷʽʻʻʿʵˀˁʽʳʶ ʴʼʿʼʺʹʸʲʽʾʽ, bool ʷʻʺʽʻˀʾʸʹʸʲ = true)
	{
		Instrument instrument = ʾʵʻʳʷʴʼʲʻʶʷ.ʾʵʻʳʷʴʼʲʻʶʷ.ʲʼʽʺʿʹʽʻʵʺʶ.instrument;
		ControllerType getControllerType = ʾʵʻʳʷʴʼʲʻʶʷ.ʾʵʻʳʷʴʼʲʻʶʷ.ʲʼʽʺʿʹʽʻʵʺʶ.controllerType.GetControllerType;
		ʹʺʿʻʲʹʻʹʹʵʸ ʲʼʽʺʿʹʽʻʵʺʶ = ʾʵʻʳʷʴʼʲʻʶʷ.ʾʵʻʳʷʴʼʲʻʶʷ.ʲʼʽʺʿʹʽʻʵʺʶ;
		ʷʽʻʻʿʵˀˁʽʳʶ.ʼˀʼʳʴˁʳʾʲˁʾ ʷʻʺʼʴʶʴʳʽʺʿ = ʲʼʽʺʿʹʽʻʵʺʶ.ʺʽʼʼʴʽʴʾʲʷˁ;
		if (ʲʼʽʺʿʹʽʻʵʺʶ.ʺʽʼʼʴʽʴʾʲʷˁ == ʷʽʻʻʿʵˀˁʽʳʶ.ʼˀʼʳʴˁʳʾʲˁʾ.ProDrums)
		{
			ʷʻʺʼʴʶʴʳʽʺʿ = ʷʽʻʻʿʵˀˁʽʳʶ.ʼˀʼʳʴˁʳʾʲˁʾ.Drums;
		}
		ʹʲʼʺʶʷʾʶʿʺʹ result = default(ʹʲʼʺʶʷʾʶʿʺʹ);

// ---- caller choice inside ʹʹʾʴʳʶʼʵʳʼʼ: generate only when the track has no written fills : ʶˀʵˁʹʿˀʶʿʲʾ.cs lines 884 to 891 of the ilspycmd output ----
			if (ˀʺʷʺʵʾʼʿʹʼʸ2.ʸʶʳʸʸʲʶʺʼʽʳ.Count == 0)
			{
				result.ʲʻʳʸʵʲˀʿʺʴʼ = ʸˀʽʸʽʳʳʸʼʾʿ(result.ˀʶʵʾʻʹʲʿˀʻˀ, ˀʺʷʺʵʾʼʿʹʼʸ2);
			}
			else
			{
				result.ʲʻʳʸʵʲˀʿʺʴʼ = ʳʷˁʸʵʾʼʷˀʲˁ(result.ˀʶʵʾʻʹʲʿˀʻˀ, ʴʼʿʼʺʹʸʲʽʾʽ, ˀʺʷʺʵʾʼʿʹʼʸ2);
			}

// ---- F1 fill generator ʸˀʽʸʽʳʳʸʼʾʿ : ʶˀʵˁʹʿˀʶʿʲʾ.cs lines 178 to 225 of the ilspycmd output ----
	public static List<ʲʺʲʴʶʴʶʻʺʷʴ> ʸˀʽʸʽʳʳʸʼʾʿ(List<ʼˀˀʸʸʶʳʼʳʸʶ> ʲʿʺʾʻʵˁʽʻʴʸ, ˀʺʷʺʵʾʼʿʹʼʸ ʴˁʶʳʳʹʽʴʳʽʵ)
	{
		List<float> ˁʹʾʲʸʿʲʷʻˁʽ = ʴˁʶʳʳʹʽʴʳʽʵ.ʷʳʷʻʸʽʽʾˁʽʵ.ʵʹˁˀʷʽʾʲˁʲʶ.ˁʹʾʲʸʿʲʷʻˁʽ;
		List<ʲʺʲʴʶʴʶʻʺʷʴ> list = new List<ʲʺʲʴʶʴʶʻʺʷʴ>();
		if (ʲʿʺʾʻʵˁʽʻʴʸ != null && ʲʿʺʾʻʵˁʽʻʴʸ.Count == 0)
		{
			return list;
		}
		int ʻʾˁʽʺˀʽʺʻʼʺ = 0;
		uint num = (uint)Mathf.RoundToInt(12f * ʴˁʶʳʳʹʽʴʳʽʵ.ʷʳʷʻʸʽʽʾˁʽʵ.ˀʳˁʻʵʺʻʳˁʸʽ / ʵʷʽʴʹʽʳʹʷʴʷ.ˀˀʽʵʶʸʸʿʷʾʲ);
		for (int i = 1; i < ˁʹʾʲʸʿʲʷʻˁʽ.Count; i += 4)
		{
			float num2 = ˁʹʾʲʸʿʲʷʻˁʽ[i - 1] + (ˁʹʾʲʸʿʲʷʻˁʽ[i] - ˁʹʾʲʸʿʲʷʻˁʽ[i - 1]) / 2f;
			float num3 = ˁʹʾʲʸʿʲʷʻˁʽ[i];
			int num4 = ʲʿʺʾʻʵˁʽʻʴʸ.ʶʲˁʿʹʵˀʺʲˁʿ(ʴˁʶʳʳʹʽʴʳʽʵ.ʷʳʷʻʸʽʽʾˁʽʵ.ʵʲˁˀʲʳʼʶʾˀʺ(num3), ʻʾˁʽʺˀʽʺʻʼʺ);
			if (num4 == -1)
			{
				i -= 3;
				continue;
			}
			ʼˀˀʸʸʶʳʼʳʸʶ ʼˀˀʸʸʶʳʼʳʸʶ2 = ʲʿʺʾʻʵˁʽʻʴʸ[num4];
			float num5 = ʼˀˀʸʸʶʳʼʳʸʶ2.ʲʸʺʺʼʷʵʵʶʵˁ - num3;
			num5 = ((num5 < 0f) ? (0f - num5) : num5);
			if (num5 > 0.25f)
			{
				i -= 3;
				continue;
			}
			ʻʾˁʽʺˀʽʺʻʼʺ = num4;
			ʼˀˀʸʸʶʳʼʳʸʶ ʼˀˀʸʸʶʳʼʳʸʶ3 = ʿʹʽʽʵʻʺʸʻʵʳ(ʼˀˀʸʸʶʳʼʳʸʶ2, ʾʲʴʵˀʷʸʺʾʹʷ: false, ʴʸʽʹʹʳʾʲʹˁʼ: true);
			if (ʼˀˀʸʸʶʳʼʳʸʶ3 != null)
			{
				ʼˀˀʸʸʶʳʼʳʸʶ3.ˀʷˁʵʹʳʿʵʼʹˀ |= ʼˀˀʸʸʶʳʼʳʸʶ.ʴʴʶʹʼʳʽʵʴʳʿ.DRUM_ACTIVATION;
				uint num6 = ʴˁʶʳʳʹʽʴʳʽʵ.ʷʳʷʻʸʽʽʾˁʽʵ.ʵʲˁˀʲʳʼʶʾˀʺ(num2);
				ʲʺʲʴʶʴʶʻʺʷʴ item = new ʲʺʲʴʶʴʶʻʺʷʴ
				{
					ʲʸʺʺʼʷʵʵʶʵˁ = num2,
					ʻʾʺʸʾʹʿʳʴʸʽ = ʴˁʶʳʳʹʽʴʳʽʵ.ʷʳʷʻʸʽʽʾˁʽʵ.ʸʳʾʸʼʹʽʾʿʸʵ(num6, ʼˀˀʸʸʶʳʼʳʸʶ3.ʵʿʵʲʽʽʽʵʸˀʾ + num),
					ʹʾʽʿʻʻʾʿʼʳʸ = ʼˀˀʸʸʶʳʼʳʸʶ3,
					ʵʿʵʲʽʽʽʵʸˀʾ = num6,
					ʷʳʽʵʳʴʼʲʴˁʲ = (int)(ʼˀˀʸʸʶʳʼʳʸʶ3.ʵʿʵʲʽʽʽʵʸˀʾ + num - num6),
					ʵˀʺʾʼʿʷʼʲʲʷ = true
				};
				list.Add(item);
			}
		}
		return list;
	}

// ---- F2 written-fill placement ʳʷˁʸʵʾʼʷˀʲˁ : ʶˀʵˁʹʿˀʶʿʲʾ.cs lines 467 to 501 of the ilspycmd output ----
	public static List<ʲʺʲʴʶʴʶʻʺʷʴ> ʳʷˁʸʵʾʼʷˀʲˁ(List<ʼˀˀʸʸʶʳʼʳʸʶ> ʲʿʺʾʻʵˁʽʻʴʸ, ʷʽʻʻʿʵˀˁʽʳʶ ʴʼʿʼʺʹʸʲʽʾʽ, ˀʺʷʺʵʾʼʿʹʼʸ ʴˁʶʳʳʹʽʴʳʽʵ)
	{
		_ = ʲʿʺʾʻʵˁʽʻʴʸ.Count;
		List<ʲʺʲʴʶʴʶʻʺʷʴ> list = new List<ʲʺʲʴʶʴʶʻʺʷʴ>(ʴˁʶʳʳʹʽʴʳʽʵ.ʸʶʳʸʸʲʶʺʼʽʳ.Count);
		uint num = (uint)Mathf.RoundToInt(6f * ʴʼʿʼʺʹʸʲʽʾʽ.ˀʳˁʻʵʺʻʳˁʸʽ / ʵʷʽʴʹʽʳʹʷʴʷ.ˀˀʽʵʶʸʸʿʷʾʲ);
		uint num2 = (uint)Mathf.RoundToInt(12f * ʴˁʶʳʳʹʽʴʳʽʵ.ʷʳʷʻʸʽʽʾˁʽʵ.ˀʳˁʻʵʺʻʳˁʸʽ / ʵʷʽʴʹʽʳʹʷʴʷ.ˀˀʽʵʶʸʸʿʷʾʲ);
		foreach (ʳʹʺˁʲʵʳˀʸʿʶ item2 in ʴˁʶʳʳʹʽʴʳʽʵ.ʸʶʳʸʸʲʶʺʼʽʳ)
		{
			if (ʴʼʿʼʺʹʸʲʽʾʽ.ʾʹʷˀʼʼˁʽʼʽʾ != null && ʴʼʿʼʺʹʸʲʽʾʽ.ʾʹʷˀʼʼˁʽʼʽʾ.ʳʿʹʺʵʹʷʼʼʸʸ == item2.ʳʿʹʺʵʹʷʼʼʸʸ)
			{
				continue;
			}
			uint num3 = item2.ʳʿʹʺʵʹʷʼʼʸʸ + item2.ʽʹʸʲˁʴʸˀʳʻʼ;
			ʼˀˀʸʸʶʳʼʳʸʶ ʼˀˀʸʸʶʳʼʳʸʶ2 = ʲʿʺʾʻʵˁʽʻʴʸ.ʷʾˀʼˀʵʹˁʻʻʷ(num3, item2.ʳʿʹʺʵʹʷʼʼʸʸ, num3 + num);
			if (ʼˀˀʸʸʶʳʼʳʸʶ2 != null)
			{
				ʼˀˀʸʸʶʳʼʳʸʶ ʼˀˀʸʸʶʳʼʳʸʶ3 = ʿʹʽʽʵʻʺʸʻʵʳ(ʼˀˀʸʸʶʳʼʳʸʶ2, ʾʲʴʵˀʷʸʺʾʹʷ: false, ʴʸʽʹʹʳʾʲʹˁʼ: true);
				if (ʼˀˀʸʸʶʳʼʳʸʶ3 != null)
				{
					ʼˀˀʸʸʶʳʼʳʸʶ3.ˀʷˁʵʹʳʿʵʼʹˀ |= ʼˀˀʸʸʶʳʼʳʸʶ.ʴʴʶʹʼʳʽʵʴʳʿ.DRUM_ACTIVATION;
					ʲʺʲʴʶʴʶʻʺʷʴ item = new ʲʺʲʴʶʴʶʻʺʷʴ
					{
						ʲʸʺʺʼʷʵʵʶʵˁ = item2.ʲʸʺʺʼʷʵʵʶʵˁ,
						ʻʾʺʸʾʹʿʳʴʸʽ = ʴˁʶʳʳʹʽʴʳʽʵ.ʷʳʷʻʸʽʽʾˁʽʵ.ʸʳʾʸʼʹʽʾʿʸʵ(item2.ʳʿʹʺʵʹʷʼʼʸʸ, ʼˀˀʸʸʶʳʼʳʸʶ3.ʵʿʵʲʽʽʽʵʸˀʾ + num2),
						ʹʾʽʿʻʻʾʿʼʳʸ = ʼˀˀʸʸʶʳʼʳʸʶ3,
						ʵʿʵʲʽʽʽʵʸˀʾ = item2.ʳʿʹʺʵʹʷʼʼʸʸ,
						ʷʳʽʵʳʴʼʲʴˁʲ = (int)(ʼˀˀʸʸʶʳʼʳʸʶ3.ʵʿʵʲʽʽʽʵʸˀʾ - item2.ʳʿʹʺʵʹʷʼʼʸʸ + num2),
						ʵˀʺʾʼʿʷʼʲʲʷ = true
					};
					list.Add(item);
				}
			}
		}
		return list;
	}

// ---- F3 pick the activation note of a chord ʶʸʲʷʵʸʻʻʸʹʸ : ʶˀʵˁʹʿˀʶʿʲʾ.cs lines 1017 to 1041 of the ilspycmd output ----
	private static ʼˀˀʸʸʶʳʼʳʸʶ ʶʸʲʷʵʸʻʻʸʹʸ(ʼˀˀʸʸʶʳʼʳʸʶ ʹʿʶʾʺʾˁʴʻˁʲ, bool ʴʸʽʹʹʳʾʲʹˁʼ = false)
	{
		ushort num = ʹʿʶʾʺʾˁʴʻˁʲ.ʵʹʴʸʽʷʺʳʵʿʳ;
		uint num2 = (uint)(num & 0x7F);
		uint num3 = (uint)(num & 0x180);
		uint num4 = 0u;
		if (num2 != 0)
		{
			num4 = ʵʼʴʿʼʷʷˀʶˁʷ.ʷʹʾˁʳʸʲʾʴʾʿ(num2);
		}
		else if (ʴʸʽʹʹʳʾʲʹˁʼ && num3 != 0)
		{
			num4 = ʵʼʴʿʼʷʷˀʶˁʷ.ʴʽʸʿʼʸˁʷʿʹʸ(num3);
		}
		ʼˀˀʸʸʶʳʼʳʸʶ.ˀˁˀʼʵʼˀʵʷʸʳ ˀˁˀʼʵʼˀʵʷʸʳ = ʹʿʶʾʺʾˁʴʻˁʲ.ʾʽʶʻʸʾʳʴʿʳʲ.ʼˀʸʹʴʻʾʶʼʷˁ();
		while (ˀˁˀʼʵʼˀʵʷʸʳ.ʿʵʷʻʵʻʷʵʶʺʼ())
		{
			ʼˀˀʸʸʶʳʼʳʸʶ ʼˀˀʸʸʶʳʼʳʸʶ2 = ˀˁˀʼʵʼˀʵʷʸʳ.ʹʺʻʴˀʴʶʲʹʳʿ;
			if (ʼˀˀʸʸʶʳʼʳʸʶ2.ʿˀʸʻʺʸʳʺʴʻʿ == num4)
			{
				return ʼˀˀʸʸʶʳʼʳʸʶ2;
			}
		}
		return null;
	}

// ---- F4 nearest chord to a tick ʶʲˁʿʹʵˀʺʲˁʿ : ʴʻˁʷʸʶʶʶʿʳʹ.cs lines 194 to 217 of the ilspycmd output ----
	public static int ʶʲˁʿʹʵˀʺʲˁʿ<T>(this IList<T> ʹʺʼʿʲʸʹʺʲʶʹ, uint ʵʻʼˀʵˁʿʽʵʷʹ, int ʻʾˁʽʺˀʽʺʻʼʺ = 0) where T : class, ʶʼˀˁʲʴʿʾʵʲʻ
	{
		int num = ʹʺʼʿʲʸʹʺʲʶʹ.ʼʿˀʹʵˁˁʹʽʸʹ(ʵʻʼˀʵˁʿʽʵʷʹ, ʻʾˁʽʺˀʽʺʻʼʺ);
		int ʻʾˁʽʺˀʽʺʻʼʺ2 = ((num == -1) ? ʻʾˁʽʺˀʽʺʻʼʺ : num);
		int num2 = ʹʺʼʿʲʸʹʺʲʶʹ.ʴʶʾˀˁʾʵʼʾʺʽ(ʵʻʼˀʵˁʿʽʵʷʹ, ʻʾˁʽʺˀʽʺʻʼʺ2, ʴʼʲʹʾʵʾʹʼʵʿ: true);
		if (num2 == -1 && num == -1)
		{
			return -1;
		}
		if (num == -1)
		{
			return num2;
		}
		if (num2 == -1)
		{
			return num;
		}
		T val = ʹʺʼʿʲʸʹʺʲʶʹ[num2];
		if (ʹʺʼʿʲʸʹʺʲʶʹ[num].ʵʿʵʲʽʽʽʵʸˀʾ - ʵʻʼˀʵˁʿʽʵʷʹ > ʵʻʼˀʵˁʿʽʵʷʹ - val.ʵʿʵʲʽʽʽʵʸˀʾ)
		{
			return num2;
		}
		return num;
	}

// ---- F4 helper: first chord strictly after a tick ʼʿˀʹʵˁˁʹʽʸʹ : ʴʻˁʷʸʶʶʶʿʳʹ.cs lines 6 to 17 of the ilspycmd output ----
	public static int ʼʿˀʹʵˁˁʹʽʸʹ<T>(this IList<T> ʹʺʼʿʲʸʹʺʲʶʹ, uint ʵʻʼˀʵˁʿʽʵʷʹ, int ʻʾˁʽʺˀʽʺʻʼʺ = 0) where T : class, ʶʼˀˁʲʴʿʾʵʲʻ
	{
		foreach (var item in EnumerableRanges.EnumerateRangeIndexed(ʹʺʼʿʲʸʹʺʲʶʹ, ʻʾˁʽʺˀʽʺʻʼʺ))
		{
			var (result, _) = item;
			if (item.Item2.ʵʿʵʲʽʽʽʵʸˀʾ > ʵʻʼˀʵˁʿʽʵʷʹ)
			{
				return result;
			}
		}
		return 0;
	}

// ---- F4 helper: last chord at or before a tick ʴʶʾˀˁʾʵʼʾʺʽ : ʴʻˁʷʸʶʶʶʿʳʹ.cs lines 140 to 150 of the ilspycmd output ----
	public static int ʴʶʾˀˁʾʵʼʾʺʽ<T>(this IList<T> ʹʺʼʿʲʸʹʺʲʶʹ, uint ʵʻʼˀʵˁʿʽʵʷʹ, int ʻʾˁʽʺˀʽʺʻʼʺ = int.MaxValue, bool ʴʼʲʹʾʵʾʹʼʵʿ = false) where T : class, ʶʼˀˁʲʴʿʾʵʲʻ
	{
		foreach (var (result, val) in EnumerableRanges.EnumerateRangeReversedIndexed(ʹʺʼʿʲʸʹʺʲʶʹ, ʻʾˁʽʺˀʽʺʻʼʺ))
		{
			if (val.ʵʿʵʲʽʽʽʵʸˀʾ < ʵʻʼˀʵˁʿʽʵʷʹ || ((val.ʵʿʵʲʽʽʽʵʸˀʾ <= ʵʻʼˀʵˁʿʽʵʷʹ) & ʴʼʲʹʾʵʾʹʼʵʿ))
			{
				return result;
			}
		}
		return -1;
	}

// ---- F5 measure-line and beat list builder ʴˀʲʸʷʶˁˀʺʶʲ (class ʵʹˁˀʷʽʾʲˁʲʶ) : ʵʹˁˀʷʽʾʲˁʲʶ.cs lines 52 to 110 of the ilspycmd output ----
	public void ʴˀʲʸʷʶˁˀʺʶʲ(ʷʽʻʻʿʵˀˁʽʳʶ ʴʼʿʼʺʹʸʲʽʾʽ)
	{
		uint num = 0u;
		uint num2 = ʴʼʿʼʺʹʸʲʽʾʽ.ʿʼʿʿʵʵʲʿʷˁʻ();
		ˀʸʼʹʽʺʸˀʷʽʵ ˀʸʼʹʽʺʸˀʷʽʵ2 = null;
		ˀʸʼʹʽʺʸˀʷʽʵ ˀʸʼʹʽʺʸˀʷʽʵ3 = ʴʼʿʼʺʹʸʲʽʾʽ.ʴʵʿʹʹʾʿʳˀʵʼ(num);
		int num3 = 0;
		int num4 = 0;
		while (num <= num2)
		{
			ˀʸʼʹʽʺʸˀʷʽʵ3 = ʴʼʿʼʺʹʸʲʽʾʽ.ʴʵʿʹʹʾʿʳˀʵʼ(num);
			uint num5 = ˀʸʼʹʽʺʸˀʷʽʵ3.ʲʲˀʵʼʿʳʸʾʾʴ / 4;
			bool flag = ˀʸʼʹʽʺʸˀʷʽʵ2 == null || (ˀʸʼʹʽʺʸˀʷʽʵ2 != null && (ˀʸʼʹʽʺʸˀʷʽʵ2.ʶʶˁʿʷʾʹʸʴʷʲ != ˀʸʼʹʽʺʸˀʷʽʵ3.ʶʶˁʿʷʾʹʸʴʷʲ || ˀʸʼʹʽʺʸˀʷʽʵ2.ʲʲˀʵʼʿʳʸʾʾʴ != ˀʸʼʹʽʺʸˀʷʽʵ3.ʲʲˀʵʼʿʳʸʾʾʴ));
			if (flag)
			{
				num4 = 0;
				num3 = 0;
			}
			if (num3 == ˀʸʼʹʽʺʸˀʷʽʵ3.ʶʶˁʿʷʾʹʸʴʷʲ)
			{
				num3 = 0;
			}
			if ((num3 == 0) | flag)
			{
				if (ˀʸʼʹʽʺʸˀʷʽʵ3.ʶʶˁʿʷʾʹʸʴʷʲ == 1 && num4 > 0)
				{
					if (ˀʸʼʹʽʺʸˀʷʽʵ3.ʲʲˀʵʼʿʳʸʾʾʴ <= 4 || num4 % num5 == 0L)
					{
						ʼʺʾʼˁʼʹˁˁʷʻ(ʴʼʿʼʺʹʸʲʽʾʽ, num, ʻʴʴʸʾʹˁʸʺʽʷ.BEAT_STRONG);
					}
					else
					{
						ʼʺʾʼˁʼʹˁˁʷʻ(ʴʼʿʼʺʹʸʲʽʾʽ, num, ʻʴʴʸʾʹˁʸʺʽʷ.BEAT_WEAK);
					}
				}
				else
				{
					if (ˀʸʼʹʽʺʸˀʷʽʵ2 != null && ˀʸʼʹʽʺʸˀʷʽʵ2.ʲʲˀʵʼʿʳʸʾʾʴ > 4)
					{
						ʶʺʾʺʾʹʻʿʽʽʸ[ʶʺʾʺʾʹʻʿʽʽʸ.Count - 1] = ʻʴʴʸʾʹˁʸʺʽʷ.BEAT_WEAK;
					}
					ʼʺʾʼˁʼʹˁˁʷʻ(ʴʼʿʼʺʹʸʲʽʾʽ, num, ʻʴʴʸʾʹˁʸʺʽʷ.MEASURE);
				}
			}
			else if (ˀʸʼʹʽʺʸˀʷʽʵ3.ʲʲˀʵʼʿʳʸʾʾʴ <= 4 || num3 % num5 == 0L)
			{
				ʼʺʾʼˁʼʹˁˁʷʻ(ʴʼʿʼʺʹʸʲʽʾʽ, num, ʻʴʴʸʾʹˁʸʺʽʷ.BEAT_STRONG);
			}
			else
			{
				ʼʺʾʼˁʼʹˁˁʷʻ(ʴʼʿʼʺʹʸʲʽʾʽ, num, ʻʴʴʸʾʹˁʸʺʽʷ.BEAT_WEAK);
			}
			num3++;
			num4++;
			num += (uint)(ʴʼʿʼʺʹʸʲʽʾʽ.ˀʳˁʻʵʺʻʳˁʸʽ * 4f / (float)ˀʸʼʹʽʺʸˀʷʽʵ3.ʲʲˀʵʼʿʳʸʾʾʴ);
			ˀʸʼʹʽʺʸˀʷʽʵ2 = ˀʸʼʹʽʺʸˀʷʽʵ3;
		}
		ʾʾʼʴˀʲʺʺʵʸʵ = ˁʹʾʲʸʿʲʷʻˁʽ.Count - 1;
	}

// ---- F6 activation-note wrapper ʿʹʽʽʵʻʺʸʻʵʳ, which F1 and F2 call with false so it reaches F3 : ʶˀʵˁʹʿˀʶʿʲʾ.cs lines 814 to 821 of the ilspycmd output ----
	private static ʼˀˀʸʸʶʳʼʳʸʶ ʿʹʽʽʵʻʺʸʻʵʳ(ʼˀˀʸʸʶʳʼʳʸʶ ʹʿʶʾʺʾˁʴʻˁʲ, bool ʾʲʴʵˀʷʸʺʾʹʷ = false, bool ʴʸʽʹʹʳʾʲʹˁʼ = false)
	{
		if (ʾʲʴʵˀʷʸʺʾʹʷ)
		{
			return ʶʺʺʵʻʵʹʾˁʷʾ(ʹʿʶʾʺʾˁʴʻˁʲ, ʴʸʽʹʹʳʾʲʹˁʼ);
		}
		return ʶʸʲʷʵʸʻʻʸʹʸ(ʹʿʶʾʺʾˁʴʻˁʲ, ʴʸʽʹʹʳʾʲʹˁʼ);
	}

// ---- F7 highest set bit ʷʹʾˁʳʸʲʾʴʾʿ (class ʵʼʴʿʼʷʷˀʶˁʷ), called at F3 : ʵʼʴʿʼʷʷˀʶˁʷ.cs lines 635 to 644 of the ilspycmd output ----
	[MethodImpl(MethodImplOptions.AggressiveInlining)]
	public static uint ʷʹʾˁʳʸʲʾʴʾʿ(uint ʿʻʹʷʹʿʽʵʼʳʿ)
	{
		ʿʻʹʷʹʿʽʵʼʳʿ |= ʿʻʹʷʹʿʽʵʼʳʿ >> 1;
		ʿʻʹʷʹʿʽʵʼʳʿ |= ʿʻʹʷʹʿʽʵʼʳʿ >> 2;
		ʿʻʹʷʹʿʽʵʼʳʿ |= ʿʻʹʷʹʿʽʵʼʳʿ >> 4;
		ʿʻʹʷʹʿʽʵʼʳʿ |= ʿʻʹʷʹʿʽʵʼʳʿ >> 8;
		ʿʻʹʷʹʿʽʵʼʳʿ |= ʿʻʹʷʹʿʽʵʼʳʿ >> 16;
		return ʿʻʹʷʹʿʽʵʼʳʿ & ~(ʿʻʹʷʹʿʽʵʼʳʿ >> 1);
	}

// ---- F7 lowest set bit ʴʽʸʿʼʸˁʷʿʹʸ (class ʵʼʴʿʼʷʷˀʶˁʷ), called at F3 : ʵʼʴʿʼʷʷˀʶˁʷ.cs lines 376 to 380 of the ilspycmd output ----
	[MethodImpl(MethodImplOptions.AggressiveInlining)]
	public static uint ʴʽʸʿʼʸˁʷʿʹʸ(uint ʿʻʹʷʹʿʽʵʼʳʿ)
	{
		return ʿʻʹʷʹʿʽʵʼʳʿ & ~(ʿʻʹʷʹʿʽʵʼʳʿ - 1);
	}

// ---- F8 the drum lane bits, enum ʾʺʸʻʼʲˀˀʽʵʽ inside class ʼˀˀʸʸʶʳʼʳʸʶ : ʼˀˀʸʸʶʳʼʳʸʶ.cs lines 31 to 45 of the ilspycmd output ----
	[Flags]
	public enum ʾʺʸʻʼʲˀˀʽʵʽ : ushort
	{
		None = 0,
		TomRed = 1,
		TomYellow = 2,
		TomBlue = 4,
		TomGreen = 8,
		CymbalYellow = 0x10,
		CymbalBlue = 0x20,
		CymbalGreen = 0x40,
		KickOrange = 0x80,
		DoubleKickOrange = 0x100,
		FiveLaneGreen = 0x200
	}

// ---- F8 the chord fields: lane, time in seconds, length in seconds, tick, length in ticks : ʼˀˀʸʸʶʳʼʳʸʶ.cs lines 218 to 226 of the ilspycmd output ----
	public ushort ʿˀʸʻʺʸʳʺʴʻʿ { get; set; }

	public float ʲʸʺʺʼʷʵʵʶʵˁ { get; set; }

	public float ʻʾʺʸʾʹʿʳʴʸʽ { get; set; }

	public uint ʵʿʵʲʽʽʽʵʸˀʾ { get; set; }

	public int ʷʳʽʵʳʴʼʲʴˁʲ { get; set; }

// ---- F8 the chord lane mask ʵʹʴʸʽʷʺʳʵʿʳ, read at F3 : ʼˀˀʸʸʶʳʼʳʸʶ.cs lines 244 to 257 of the ilspycmd output ----
	public ushort ʵʹʴʸʽʷʺʳʵʿʳ
	{
		get
		{
			ushort num = 0;
			ˀˁˀʼʵʼˀʵʷʸʳ ˀˁˀʼʵʼˀʵʷʸʳ2 = ʾʽʶʻʸʾʳʴʿʳʲ.ʼˀʸʹʴʻʾʶʼʷˁ();
			while (ˀˁˀʼʵʼˀʵʷʸʳ2.ʿʵʷʻʵʻʷʵʶʺʼ())
			{
				ʼˀˀʸʸʶʳʼʳʸʶ ʼˀˀʸʸʶʳʼʳʸʶ2 = ˀˁˀʼʵʼˀʵʷʸʳ2.ʹʺʻʴˀʴʶʲʹʳʿ;
				num |= ʼˀˀʸʸʶʳʼʳʸʶ2.ʿˀʸʻʺʸʳʺʴʻʿ;
			}
			return num;
		}
	}

// ---- F8 the chord constructor : ʼˀˀʸʸʶʳʼʳʸʶ.cs lines 349 to 358 of the ilspycmd output ----
	public ʼˀˀʸʸʶʳʼʳʸʶ(float ʲʶʹʸʿʽʿʺʹˁʵ, float ʽʹʸʲˁʴʸˀʳʻʼ, ushort ʶʻʼʴʼʻʼʽʼʿʻ, ʿʿʻʼʵʹʶˁʻʳʽ.ʸˁʳʷʼʿʼʼʿˁʺ ʺˁʼʷʷʸʹʻʽʻʺ, ʴʴʶʹʼʳʽʵʴʳʿ ˀʷˁʵʹʳʿʵʼʹˀ, uint ˁʵʸʼʺʳʾʽʾʴʽ, int ʽˀʺʻʺʾʽʴʻʽˀ)
	{
		ʲʸʺʺʼʷʵʵʶʵˁ = ʲʶʹʸʿʽʿʺʹˁʵ;
		ʻʾʺʸʾʹʿʳʴʸʽ = ʽʹʸʲˁʴʸˀʳʻʼ;
		ʿˀʸʻʺʸʳʺʴʻʿ = ʶʻʼʴʼʻʼʽʼʿʻ;
		this.ʺˁʼʷʷʸʹʻʽʻʺ = ʺˁʼʷʷʸʹʻʽʻʺ;
		this.ˀʷˁʵʹʳʿʵʼʹˀ = ˀʷˁʵʹʳʿʵʼʹˀ;
		ʵʿʵʲʽʽʽʵʸˀʾ = ˁʵʸʼʺʳʾʽʾʴʽ;
		ʷʳʽʵʳʴʼʲʴˁʲ = ʽˀʺʻʺʾʽʴʻʽˀ;
	}

// ---- F8 the chord takes its time from the parsed note : ʶˀʵˁʹʿˀʶʿʲʾ.cs line 700 of the ilspycmd output ----
			ʼˀˀʸʸʶʳʼʳʸʶ ʼˀˀʸʸʶʳʼʳʸʶ3 = new ʼˀˀʸʸʶʳʼʳʸʶ(item.ʲʸʺʺʼʷʵʵʶʵˁ, ʽʹʸʲˁʴʸˀʳʻʼ, num2, item.ˀʲʺʶʼˀʼʾʶʲʵ, ʴʴʶʹʼʳʽʵʴʳʿ, item.ʳʿʹʺʵʹʷʼʼʸʸ, (int)item.ʵʹˀʽʺʼˁʴʼʵʺ);

// ---- F8 the lane bits by instrument ʺʷʸˀʷʴʿʲʵʼʾ : ʶˀʵˁʹʿˀʶʿʲʾ.cs lines 227 to 248 of the ilspycmd output ----
	private static ushort ʺʷʸˀʷʴʿʲʵʼʾ(ʿʿʻʼʵʹʶˁʻʳʽ ʹʿʶʾʺʾˁʴʻˁʲ, Instrument ʷʻʺʼʴʶʴʳʽʺʿ)
	{
		switch (ʷʻʺʼʴʶʴʳʽʺʿ)
		{
		case Instrument.GHLGuitar:
		case Instrument.GHLBass:
		case Instrument.GHLRhythm:
		case Instrument.GHLGuitarCoop:
			return ʷˀʹʶʺʾʻˀʼʴʸ(ʹʿʶʾʺʾˁʴʻˁʲ);
		case Instrument.Guitar:
		case Instrument.Bass:
		case Instrument.Rhythm:
		case Instrument.GuitarCoop:
		case Instrument.Keys:
			return ʼʺˁˀʷʶʺʶʼˀʷ(ʹʿʶʾʺʾˁʴʻˁʲ);
		case Instrument.Drums:
		case Instrument.ProDrums:
			return (ushort)ʴʺʵʽʻʷʴˀʸʹʺ(ʹʿʶʾʺʾˁʴʻˁʲ);
		default:
			return ʼʺˁˀʷʶʺʶʼˀʷ(ʹʿʶʾʺʾˁʴʻˁʲ);
		}
	}

// ---- F8 the drum note kind to lane bit ʴʺʵʽʻʷʴˀʸʹʺ : ʶˀʵˁʹʿˀʶʿʲʾ.cs lines 523 to 539 of the ilspycmd output ----
	private static ʼˀˀʸʸʶʳʼʳʸʶ.ʾʺʸʻʼʲˀˀʽʵʽ ʴʺʵʽʻʷʴˀʸʹʺ(ʿʿʻʼʵʹʶˁʻʳʽ ʼʸʿʹʳʾʳˁʽʵʽ)
	{
		return ʼʸʿʹʳʾʳˁʽʵʽ.ʽʺʼʸʲʷʸʸʺʺʿ switch
		{
			ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.DOUBLEKICK_ORANGE => ʼˀˀʸʸʶʳʼʳʸʶ.ʾʺʸʻʼʲˀˀʽʵʽ.DoubleKickOrange,
			ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.KICK_ORANGE => ʼˀˀʸʸʶʳʼʳʸʶ.ʾʺʸʻʼʲˀˀʽʵʽ.KickOrange,
			ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.TOM_RED => ʼˀˀʸʸʶʳʼʳʸʶ.ʾʺʸʻʼʲˀˀʽʵʽ.TomRed,
			ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.TOM_YELLOW => ʼˀˀʸʸʶʳʼʳʸʶ.ʾʺʸʻʼʲˀˀʽʵʽ.TomYellow,
			ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.TOM_BLUE => ʼˀˀʸʸʶʳʼʳʸʶ.ʾʺʸʻʼʲˀˀʽʵʽ.TomBlue,
			ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.TOM_GREEN => ʼˀˀʸʸʶʳʼʳʸʶ.ʾʺʸʻʼʲˀˀʽʵʽ.TomGreen,
			ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.CYM_YELLOW => ʼˀˀʸʸʶʳʼʳʸʶ.ʾʺʸʻʼʲˀˀʽʵʽ.CymbalYellow,
			ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.CYM_BLUE => ʼˀˀʸʸʶʳʼʳʸʶ.ʾʺʸʻʼʲˀˀʽʵʽ.CymbalBlue,
			ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.CYM_GREEN => ʼˀˀʸʸʶʳʼʳʸʶ.ʾʺʸʻʼʲˀˀʽʵʽ.CymbalGreen,
			ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.FIVELANE_DRUM_GREEN => ʼˀˀʸʸʶʳʼʳʸʶ.ʾʺʸʻʼʲˀˀʽʵʽ.FiveLaneGreen,
			_ => ʼˀˀʸʸʶʳʼʳʸʶ.ʾʺʸʻʼʲˀˀʽʵʽ.None,
		};
	}

// ---- F8 a .mid drum note number to note kind (class ʷʹʿʴʿʿˁʳʳʸʳ): yellow, blue and green are cymbals by default : ʷʹʿʴʿʿˁʳʳʸʳ.cs lines 3 to 45 of the ilspycmd output ----
	public static ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ ʸʴʶʳʻˁʶˀʻʸˀ(byte ʽʼʼʳˁʸʲʵʷʻʽ)
	{
		switch (ʽʼʼʳˁʸʲʵʷʻʽ)
		{
		case 59:
		case 71:
		case 83:
		case 95:
			return ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.DOUBLEKICK_ORANGE;
		case 60:
		case 72:
		case 84:
		case 96:
			return ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.KICK_ORANGE;
		case 61:
		case 73:
		case 85:
		case 97:
			return ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.TOM_RED;
		case 62:
		case 74:
		case 86:
		case 98:
			return ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.CYM_YELLOW;
		case 63:
		case 75:
		case 87:
		case 99:
			return ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.CYM_BLUE;
		case 64:
		case 76:
		case 88:
		case 100:
			return ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.CYM_GREEN;
		case 65:
		case 77:
		case 89:
		case 101:
			return ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.FIVELANE_DRUM_GREEN;
		default:
			return ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.NONE;
		}
	}

// ---- F8 .mid tom markers 110 to 112 turn a cymbal into a tom (inside the .mid reader) : ʻˀʵˁʺʷʳˀʷʻʳ.cs lines 209 to 234 of the ilspycmd output ----
					switch (ˀʺʷʺʵʾʼʿʹʼʸ2.ʲʿʺʾʻʵˁʽʻʴʸ[k].ʽʺʼʸʲʷʸʸʺʺʿ)
					{
					case ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.CYM_YELLOW:
						if ((byte)list3[j].NoteNumber == 110)
						{
							ˀʺʷʺʵʾʼʿʹʼʸ2.ʲʿʺʾʻʵˁʽʻʴʸ[k].ʽʺʼʸʲʷʸʸʺʺʿ = ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.TOM_YELLOW;
							ˀʺʷʺʵʾʼʿʹʼʸ2.ʲʿʺʾʻʵˁʽʻʴʸ[k].ˀʷˁʵʹʳʿʵʼʹˀ ^= ʿʿʻʼʵʹʶˁʻʳʽ.ʷʻʵʿʺʼʹʵˀʲʼ.CYMBAL;
							ˀʺʷʺʵʾʼʿʹʼʸ2.ʲʿʺʾʻʵˁʽʻʴʸ[k].ˀʷˁʵʹʳʿʵʼʹˀ |= ʿʿʻʼʵʹʶˁʻʳʽ.ʷʻʵʿʺʼʹʵˀʲʼ.TOM;
						}
						break;
					case ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.CYM_BLUE:
						if ((byte)list3[j].NoteNumber == 111)
						{
							ˀʺʷʺʵʾʼʿʹʼʸ2.ʲʿʺʾʻʵˁʽʻʴʸ[k].ʽʺʼʸʲʷʸʸʺʺʿ = ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.TOM_BLUE;
							ˀʺʷʺʵʾʼʿʹʼʸ2.ʲʿʺʾʻʵˁʽʻʴʸ[k].ˀʷˁʵʹʳʿʵʼʹˀ ^= ʿʿʻʼʵʹʶˁʻʳʽ.ʷʻʵʿʺʼʹʵˀʲʼ.CYMBAL;
							ˀʺʷʺʵʾʼʿʹʼʸ2.ʲʿʺʾʻʵˁʽʻʴʸ[k].ˀʷˁʵʹʳʿʵʼʹˀ |= ʿʿʻʼʵʹʶˁʻʳʽ.ʷʻʵʿʺʼʹʵˀʲʼ.TOM;
						}
						break;
					case ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.CYM_GREEN:
						if ((byte)list3[j].NoteNumber == 112)
						{
							ˀʺʷʺʵʾʼʿʹʼʸ2.ʲʿʺʾʻʵˁʽʻʴʸ[k].ʽʺʼʸʲʷʸʸʺʺʿ = ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.TOM_GREEN;
							ˀʺʷʺʵʾʼʿʹʼʸ2.ʲʿʺʾʻʵˁʽʻʴʸ[k].ˀʷˁʵʹʳʿʵʼʹˀ ^= ʿʿʻʼʵʹʶˁʻʳʽ.ʷʻʵʿʺʼʹʵˀʲʼ.CYMBAL;
							ˀʺʷʺʵʾʼʿʹʼʸ2.ʲʿʺʾʻʵˁʽʻʴʸ[k].ˀʷˁʵʹʳʿʵʼʹˀ |= ʿʿʻʼʵʹʶˁʻʳʽ.ʷʻʵʿʺʼʹʵˀʲʼ.TOM;
						}
						break;

// ---- F8 .chart cymbal markers 66 to 68 turn a tom into a cymbal (inside the .chart track reader) : ˀʺʷʺʵʾʼʿʹʼʸ.cs lines 271 to 297 of the ilspycmd output ----
							case 66u:
								if (item2.ʽʺʼʸʲʷʸʸʺʺʿ == ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.TOM_YELLOW)
								{
									item2.ʽʺʼʸʲʷʸʸʺʺʿ = ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.CYM_YELLOW;
									item2.ˀʷˁʵʹʳʿʵʼʹˀ ^= ʿʿʻʼʵʹʶˁʻʳʽ.ʷʻʵʿʺʼʹʵˀʲʼ.TOM;
									item2.ˀʷˁʵʹʳʿʵʼʹˀ |= ʿʿʻʼʵʹʶˁʻʳʽ.ʷʻʵʿʺʼʹʵˀʲʼ.CYMBAL;
									ʼˁʶʻʸʵʼʹʸʿʺ = true;
								}
								break;
							case 67u:
								if (item2.ʽʺʼʸʲʷʸʸʺʺʿ == ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.TOM_BLUE)
								{
									item2.ʽʺʼʸʲʷʸʸʺʺʿ = ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.CYM_BLUE;
									item2.ˀʷˁʵʹʳʿʵʼʹˀ ^= ʿʿʻʼʵʹʶˁʻʳʽ.ʷʻʵʿʺʼʹʵˀʲʼ.TOM;
									item2.ˀʷˁʵʹʳʿʵʼʹˀ |= ʿʿʻʼʵʹʶˁʻʳʽ.ʷʻʵʿʺʼʹʵˀʲʼ.CYMBAL;
									ʼˁʶʻʸʵʼʹʸʿʺ = true;
								}
								break;
							case 68u:
								if (item2.ʽʺʼʸʲʷʸʸʺʺʿ == ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.TOM_GREEN)
								{
									item2.ʽʺʼʸʲʷʸʸʺʺʿ = ʿʿʻʼʵʹʶˁʻʳʽ.ˁʲʵʾˀˀˁʲʳʾʺ.CYM_GREEN;
									item2.ˀʷˁʵʹʳʿʵʼʹˀ ^= ʿʿʻʼʵʹʶˁʻʳʽ.ʷʻʵʿʺʼʹʵˀʲʼ.TOM;
									item2.ˀʷˁʵʹʳʿʵʼʹˀ |= ʿʿʻʼʵʹʶˁʻʳʽ.ʷʻʵʿʺʼʹʵˀʲʼ.CYMBAL;
									ʼˁʶʻʸʵʼʹʸʿʺ = true;
								}
								break;

// ---- F9 class ʷʽʻʻʿʵˀˁʽʳʶ fields: the timing resolution and the chart resolution : ʷʽʻʻʿʵˀˁʽʳʶ.cs lines 69 to 71 of the ilspycmd output ----
	public float ˀʻʺʷʾʿʿʻʼʲʿ = 192f;

	public float ˀʳˁʻʵʺʻʳˁʸʽ = 192f;

// ---- F9 .chart header: both resolutions set, the timing one times the song speed : ʷʽʻʻʿʵˀˁʽʳʶ.cs lines 281 to 285 of the ilspycmd output ----
				if (dictionary.ʻʶʸʻʼʲʻʹʹʺʾ<int>("Resolution", out var ʴʿʳʳʿʲˁʾʲʽʾ5))
				{
					ˀʻʺʷʾʿʿʻʼʲʿ = (float)ʴʿʳʳʿʲˁʾʲʽʾ5 * ʹʺʽˁʽˁˀʼʶʷʼ.ʳʷʴʺʳʹʷʾʲʷʾ.GetFloatPercent;
					ˀʳˁʻʵʺʻʳˁʸʽ = ʴʿʳʳʿʲˁʾʲʽʾ5;
				}

// ---- F9 .mid reader: both resolutions set the same way : ʻˀʵˁʺʷʳˀʷʻʳ.cs lines 449 to 450 of the ilspycmd output ----
		ʷʽʻʻʿʵˀˁʽʳʶ2.ˀʳˁʻʵʺʻʳˁʸʽ = ((TicksPerQuarterNoteTimeDivision)ʺʷʸʼʲˁʿʿˀʾʹ.TimeDivision).TicksPerQuarterNote;
		ʷʽʻʻʿʵˀˁʽʳʶ2.ˀʻʺʷʾʿʿʻʼʲʿ = ʷʽʻʻʿʵˀˁʽʳʶ2.ˀʳˁʻʵʺʻʳˁʸʽ * ʹʺʽˁʽˁˀʼʶʷʼ.ʳʷʴʺʳʹʷʾʲʷʾ.GetFloatPercent;

// ---- F9 the tempo class ʼʹʵˁʸʳʾʺˁʳʽ: milli-BPM and its start second : ʼʹʵˁʸʳʾʺˁʳʽ.cs lines 1 to 12 of the ilspycmd output ----
public class ʼʹʵˁʸʳʾʺˁʳʽ : ʻʺʵˀʸˁʻʼʷˀʴ
{
	public uint ʶʲʵʻˁʻʺʴʿʺʵ;

	public float ʸʼʻʳʷʾʹʿˁʴʸ;

	public ʼʹʵˁʸʳʾʺˁʳʽ(uint ʴʼʵʹʸʻʴʻʾʼʼ = 0u, uint ʲʺˁʽˁʺʷʵʾʾʺ = 120000u)
		: base(ʴʼʵʹʸʻʴʻʾʼʼ)
	{
		ʶʲʵʻˁʻʺʴʿʺʵ = ʲʺˁʽˁʺʷʵʾʾʺ;
	}
}

// ---- F9 the base class of chart objects ʼʺˀʲʺˀʼˁʽʸʸ: tick and its second : ʼʺˀʲʺˀʼˁʽʸʸ.cs lines 4 to 13 of the ilspycmd output ----
public abstract class ʼʺˀʲʺˀʼˁʽʸʸ
{
	[NonSerialized]
	public ʷʽʻʻʿʵˀˁʽʳʶ ʴʼʿʼʺʹʸʲʽʾʽ;

	public uint ʳʿʹʺʵʹʷʼʼʸʸ;

	public const int ˁʶʳʽˀʿʺʵʴʼˁ = -1;

	public float ʲʸʺʺʼʷʵʵʶʵˁ => ʴʼʿʼʺʹʸʲʽʾʽ.ʹʴʶʸʾʲʶˀʼʺʷ(ʳʿʹʺʵʹʷʼʼʸʸ);

// ---- F9 tick to seconds ʹʴʶʸʾʲʶˀʼʺʷ : ʷʽʻʻʿʵˀˁʽʳʶ.cs lines 363 to 367 of the ilspycmd output ----
	public float ʹʴʶʸʾʲʶˀʼʺʷ(uint ʳʿʹʺʵʹʷʼʼʸʸ)
	{
		ʼʹʵˁʸʳʾʺˁʳʽ ʼʹʵˁʸʳʾʺˁʳʽ2 = ʼʿʵʸʾʼʸʿʼˀʺ.ʶʷʽʺʿʹʻʼʴʶʽ(ʳʿʹʺʵʹʷʼʼʸʸ, (ʼʹʵˁʸʳʾʺˁʳʽ bpm) => bpm.ʳʿʹʺʵʹʷʼʼʸʸ);
		return ʼʹʵˁʸʳʾʺˁʳʽ2.ʸʼʻʳʷʾʹʿˁʴʸ + ʽˀˀʻʹʶˁʽˁˁʽ(ʼʹʵˁʸʳʾʺˁʳʽ2.ʳʿʹʺʵʹʷʼʼʸʸ, ʳʿʹʺʵʹʷʼʼʸʸ, ˀʻʺʷʾʿʿʻʼʲʿ, (float)ʼʹʵˁʸʳʾʺˁʳʽ2.ʶʲʵʻˁʻʺʴʿʺʵ / 1000f);
	}

// ---- F9 ticks to seconds at one tempo ʽˀˀʻʹʶˁʽˁˁʽ : ʷʽʻʻʿʵˀˁʽʳʶ.cs lines 375 to 378 of the ilspycmd output ----
	public static float ʽˀˀʻʹʶˁʽˁˁʽ(uint ʶˀʲʿʲʴʶʷˀʿʺ, uint ʼˀʾʲʺˀʾʾʷʹʹ, float ˀʻʺʷʾʿʿʻʼʲʿ, float ˀˁʿʴʺˀʻʵˁʹʼ)
	{
		return (float)(ʼˀʾʲʺˀʾʾʷʹʹ - ʶˀʲʿʲʴʶʷˀʿʺ) / ˀʻʺʷʾʿʿʻʼʲʿ * 60f / ˀˁʿʴʺˀʻʵˁʹʼ;
	}

// ---- F9 the last item at or before a tick ʶʷʽʺʿʹʻʼʴʶʽ (class ʿʺʶʹʲʳʶʺʶʸʶ), the tempo lookup of tick to seconds : ʿʺʶʹʲʳʶʺʶʸʶ.cs lines 23 to 63 of the ilspycmd output ----
	internal static T ʵʽʺʽʼˀʴʿˀʵˀ<T, TList>(ʻʼʷʵˀʶˁʼʳʳʵ<T, TList> ʶʺˀʽˁʸʵˀʶˁʾ) where TList : IList<T>
	{
		IList<T> ʶʿʿʸʹʵʺʵʵˁʺ = ʶʺˀʽˁʸʵˀʶˁʾ.ʶʿʿʸʹʵʺʵʵˁʺ;
		uint ʳʿʹʺʵʹʷʼʼʸʸ = ʶʺˀʽˁʸʵˀʶˁʾ.ʳʿʹʺʵʹʷʼʼʸʸ;
		Func<T, uint> ʼʶʶˀʻʸʻˀʸʷˀ = ʶʺˀʽˁʸʵˀʶˁʾ.ʼʶʶˀʻʸʻˀʸʷˀ;
		int num = 0;
		int num2 = ʶʿʿʸʹʵʺʵʵˁʺ.Count - 1;
		if (ʶʿʿʸʹʵʺʵʵˁʺ.Count == 0)
		{
			return default(T);
		}
		int num3;
		while (true)
		{
			num3 = (num + num2) / 2;
			uint num4 = ʼʶʶˀʻʸʻˀʸʷˀ(ʶʿʿʸʹʵʺʵʵˁʺ[num3]);
			if (num2 - num <= 1 || num4 == ʳʿʹʺʵʹʷʼʼʸʸ)
			{
				break;
			}
			if (num4 < ʳʿʹʺʵʹʷʼʼʸʸ)
			{
				num = num3;
			}
			else
			{
				num2 = num3;
			}
		}
		if (num3 < ʶʿʿʸʹʵʺʵʵˁʺ.Count - 1 && ʼʶʶˀʻʸʻˀʸʷˀ(ʶʿʿʸʹʵʺʵʵˁʺ[num3 + 1]) <= ʳʿʹʺʵʹʷʼʼʸʸ)
		{
			return ʶʿʿʸʹʵʺʵʵˁʺ[num3 + 1];
		}
		return ʶʿʿʸʹʵʺʵʵˁʺ[num3];
	}

	[MethodImpl(MethodImplOptions.AggressiveInlining)]
	public static T ʶʷʽʺʿʹʻʼʴʶʽ<T, TList>(this TList ʶʿʿʸʹʵʺʵʵˁʺ, uint ʳʿʹʺʵʹʷʼʼʸʸ, Func<T, uint> ʼʶʶˀʻʸʻˀʸʷˀ) where TList : IList<T>
	{
		return ʵʽʺʽʼˀʴʿˀʵˀ(new ʻʼʷʵˀʶˁʼʳʳʵ<T, TList>(ʶʿʿʸʹʵʺʵʵˁʺ, ʳʿʹʺʵʹʷʼʼʸʸ, ʼʶʶˀʻʸʻˀʸʷˀ));
	}

// ---- F9 seconds between two ticks ʸʳʾʸʼʹʽʾʿʸʵ : ʷʽʻʻʿʵˀˁʽʳʶ.cs lines 628 to 635 of the ilspycmd output ----
	public float ʸʳʾʸʼʹʽʾʿʸʵ(uint ʶˀʲʿʲʴʶʷˀʿʺ, uint ʼˀʾʲʺˀʾʾʷʹʹ)
	{
		if (ʶˀʲʿʲʴʶʷˀʿʺ != ʼˀʾʲʺˀʾʾʷʹʹ)
		{
			return ʹʴʶʸʾʲʶˀʼʺʷ(ʼˀʾʲʺˀʾʾʷʹʹ) - ʹʴʶʸʾʲʶˀʼʺʷ(ʶˀʲʿʲʴʶʷˀʿʺ);
		}
		return 0f;
	}

// ---- F9 seconds to tick ʵʲˁˀʲʳʼʶʾˀʺ : ʷʽʻʻʿʵˀˁʽʳʶ.cs lines 804 to 820 of the ilspycmd output ----
	public uint ʵʲˁˀʲʳʼʶʾˀʺ(float ʲʶʹʸʿʽʿʺʹˁʵ, bool ˀʻʹʽʹʸʶʾʽʸʵ = true)
	{
		if (ʲʶʹʸʿʽʿʺʹˁʵ < 0f)
		{
			ʲʶʹʸʿʽʿʺʹˁʵ = 0f;
		}
		ʼʹʵˁʸʳʾʺˁʳʽ ʼʹʵˁʸʳʾʺˁʳʽ2 = ʼʿʵʸʾʼʸʿʼˀʺ[0];
		foreach (ʼʹʵˁʸʳʾʺˁʳʽ item in ʼʿʵʸʾʼʸʿʼˀʺ)
		{
			if (item.ʸʼʻʳʷʾʹʿˁʴʸ >= ʲʶʹʸʿʽʿʺʹˁʵ)
			{
				break;
			}
			ʼʹʵˁʸʳʾʺˁʳʽ2 = item;
		}
		return ʼʹʵˁʸʳʾʺˁʳʽ2.ʳʿʹʺʵʹʷʼʼʸʸ + ˀʽʹʷʺˁʵʹˁʻʲ(ʼʹʵˁʸʳʾʺˁʳʽ2.ʸʼʻʳʷʾʹʿˁʴʸ, ʲʶʹʸʿʽʿʺʹˁʵ, ˀʻʺʷʾʿʿʻʼʲʿ, (float)ʼʹʵˁʸʳʾʺˁʳʽ2.ʶʲʵʻˁʻʺʴʿʺʵ / 1000f);
	}

// ---- F9 seconds to ticks at one tempo ˀʽʹʷʺˁʵʹˁʻʲ : ʷʽʻʻʿʵˀˁʽʳʶ.cs lines 861 to 864 of the ilspycmd output ----
	public static uint ˀʽʹʷʺˁʵʹˁʻʲ(float ʵʻʾʷʶʺʲʺʴʾʹ, float ʸʹʳʷʳʹʳʿʵʿʼ, float ˀʻʺʷʾʿʿʻʼʲʿ, float ˀˁʿʴʺˀʻʵˁʹʼ)
	{
		return (uint)Math.Round((ʸʹʳʷʳʹʳʿʵʿʼ - ʵʻʾʷʶʺʲʺʴʾʹ) * ˀˁʿʴʺˀʻʵˁʹʼ / 60f * ˀʻʺʷʾʿʿʻʼʲʿ);
	}

// ---- F10 the chart last tick ʿʼʿʿʵʵʲʿʷˁʻ, which F5 walks to : ʷʽʻʻʿʵˀˁʽʳʶ.cs lines 995 to 1008 of the ilspycmd output ----
	public uint ʿʼʿʿʵʵʲʿʷˁʻ()
	{
		uint num = 0u;
		ˀʺʷʺʵʾʼʿʹʼʸ[] array = ʲʽʻʼʻʾʳʼʳʻʽ;
		for (int i = 0; i < array.Length; i++)
		{
			uint num2 = array[i].ʿʼʿʿʵʵʲʿʷˁʻ();
			if (num2 > num)
			{
				num = num2;
			}
		}
		return num + (uint)ˀʻʺʷʾʿʿʻʼʲʿ;
	}

// ---- F10 a track last tick ʿʼʿʿʵʵʲʿʷˁʻ (class ˀʺʷʺʵʾʼʿʹʼʸ) : ˀʺʷʺʵʾʼʿʹʼʸ.cs lines 381 to 397 of the ilspycmd output ----
	public uint ʿʼʿʿʵʵʲʿʷˁʻ()
	{
		if (ʲʿʺʾʻʵˁʽʻʴʸ == null || ʲʿʺʾʻʵˁʽʻʴʸ.Count == 0)
		{
			return 0u;
		}
		uint num = 0u;
		foreach (ʿʿʻʼʵʹʶˁʻʳʽ item in ʲʿʺʾʻʵˁʽʻʴʸ)
		{
			uint num2 = item.ʳʿʹʺʵʹʷʼʼʸʸ + item.ʵʹˀʽʺʼˁʴʼʵʺ;
			if (num2 > num)
			{
				num = num2;
			}
		}
		return num;
	}

// ---- F11 add one line ʼʺʾʼˁʼʹˁˁʷʻ (class ʵʹˁˀʷʽʾʲˁʲʶ), called by F5 : ʵʹˁˀʷʽʾʲˁʲʶ.cs lines 28 to 36 of the ilspycmd output ----
	private void ʼʺʾʼˁʼʹˁˁʷʻ(ʷʽʻʻʿʵˀˁʽʳʶ ʴʼʿʼʺʹʸʲʽʾʽ, uint ʶʲʸʴʷʷʲʸˁʵʿ, ʻʴʴʸʾʹˁʸʺʽʷ ˀʲʺʶʼˀʼʾʶʲʵ)
	{
		ʻʴʴʵʶʻʸʲˁʶʿ.Add(ʴʼʿʼʺʹʸʲʽʾʽ.ʹʴʶʸʾʲʶˀʼʺʷ(ʶʲʸʴʷʷʲʸˁʵʿ));
		ʶʺʾʺʾʹʻʿʽʽʸ.Add(ˀʲʺʶʼˀʼʾʶʲʵ);
		if (ˀʲʺʶʼˀʼʾʶʲʵ == ʻʴʴʸʾʹˁʸʺʽʷ.MEASURE)
		{
			ˁʹʾʲʸʿʲʷʻˁʽ.Add(ʻʴʴʵʶʻʸʲˁʶʿ.Last());
		}
	}

// ---- F12 StrikeCore.EnumerableRanges.IndexedRangeEnumerator, the walk the F4 helpers use (StrikeCore.dll) : EnumerableRanges.cs lines 78 to 141 of the ilspycmd output ----
	public struct IndexedRangeEnumerator<T>
	{
		private int currentPos;

		private readonly int start;

		private readonly RangeDirection direction;

		private readonly IList<T> input;

		private readonly int minMaxIndex;

		private readonly bool isValid;

		public (int, T) Current => (currentPos, input[currentPos]);

		public IndexedRangeEnumerator(IList<T> input, int start, int end, RangeDirection dir)
		{
			this.input = input;
			int num = input.Count - 1;
			this.start = Numerics.Clamp(start, 0, num);
			direction = dir;
			if (start >= 0 && end >= 0 && start <= num)
			{
				isValid = true;
			}
			else
			{
				isValid = false;
			}
			currentPos = ((dir == RangeDirection.Forward) ? (this.start - 1) : (this.start + 1));
			minMaxIndex = Numerics.Clamp(end, 0, num);
		}

		public bool MoveNext()
		{
			if (!isValid)
			{
				return false;
			}
			if (direction == RangeDirection.Forward)
			{
				currentPos++;
				if (currentPos <= minMaxIndex)
				{
					return true;
				}
			}
			if (direction == RangeDirection.Reverse)
			{
				currentPos--;
				if (currentPos >= minMaxIndex)
				{
					return true;
				}
			}
			return false;
		}

		public IndexedRangeEnumerator<T> GetEnumerator()
		{
			return this;
		}
	}

// ---- F12 StrikeCore.EnumerableRanges.EnumerateRangeIndexed and EnumerateRangeReversedIndexed (StrikeCore.dll) : EnumerableRanges.cs lines 153 to 160 of the ilspycmd output ----
	public static IndexedRangeEnumerator<T> EnumerateRangeIndexed<T>(IList<T> input, int startIndex, int endIndex = int.MaxValue)
	{
		return new IndexedRangeEnumerator<T>(input, startIndex, endIndex, RangeDirection.Forward);
	}

	public static IndexedRangeEnumerator<T> EnumerateRangeReversedIndexed<T>(IList<T> input, int startIndex = int.MaxValue, int endIndex = 0)
	{
		return new IndexedRangeEnumerator<T>(input, startIndex, endIndex, RangeDirection.Reverse);
	}

// ---- F13 UnityEngine.Mathf.RoundToInt (UnityEngine.CoreModule.dll) : Mathf.cs lines 135 to 138 of the ilspycmd -t output ----
	public static int RoundToInt(float f)
	{
		return (int)Math.Round(f);
	}

